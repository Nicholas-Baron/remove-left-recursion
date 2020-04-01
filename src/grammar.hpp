#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <ios>
#include <limits>
#include <map>
#include <vector>

// This class stores the grammar, while allowing some higher level manipulations
// on it. The nonterminals are the positive numbers, while the terminals are the
// negative numbers.
class grammar {
	// TODO: Write replace_rule

   public:
	static constexpr int  rule_sep		= 0;
	static constexpr char rule_sep_char = '|';

	static grammar empty() { return grammar{}; }
	static grammar parse_from_file(const std::string & data);

	// Returns each rule as its own vector
	std::vector<std::vector<int>> rule_matrix(int nonterminal) const;

	auto nonterminal_count() const { return rules.size(); }
	auto terminal_count() const {
		return symbols.size() - (1 + nonterminal_count());
	}

	int get_nonterminal(char symbol);

	int get_terminal(char symbol);

	bool has_empty_production(int nonterminal) const;

	bool has_any_empty_production() const {
		for (const auto nonterm : this->nonterminals())
			if (this->has_empty_production(nonterm)) return true;

		return false;
	}

	bool has_any_cycle() const { return this->cyclic_path().size() > 0; }

	// Prints one possible cycle path
	// Empty if could not find one
	std::vector<int> cyclic_path() const;

	bool using_symbol(char symbol) const;

	bool is_nonterminal_symbol(char symbol) const {
		return isupper(symbol) and this->using_symbol(symbol);
	}

	bool is_terminal_symbol(char symbol) const {
		return islower(symbol) and this->using_symbol(symbol);
	}

	std::vector<int> nonterminals() const {
		std::vector<int> to_ret{};
		for (const auto & entry : symbols) {
			if (entry.first > 0) { to_ret.push_back(entry.first); }
		}
		return to_ret;
	}

	std::vector<int> terminals() const {
		std::vector<int> to_ret{};
		for (const auto & entry : symbols) {
			if (entry.first < 0) { to_ret.push_back(entry.first); }
		}
		return to_ret;
	}

	std::vector<char> symbol_list() const {
		std::vector<char> to_ret;
		to_ret.reserve(symbols.size());
		for (const auto & [_, letter] : symbols) {
			if (letter != rule_sep_char) to_ret.emplace_back(letter);
		}
		return to_ret;
	}

	std::map<int, char> nonterminal_keys() const {
		std::map<int, char> to_ret;
		auto				nonterms = this->nonterminals();
		for (const auto nonterm : nonterms)
			to_ret.emplace(nonterm, symbols.at(nonterm));

		return to_ret;
	}

	std::map<int, char> terminal_keys() const {
		std::map<int, char> to_ret;
		auto				terms = this->terminals();
		for (const auto term : terms) to_ret.emplace(term, symbols.at(term));

		return to_ret;
	}

	// Returns true if the rule was successfully added
	bool add_rule(char symbol, std::vector<int> && rule) {
		if (this->is_nonterminal_symbol(symbol)) {
			const auto nonterm = get_nonterminal(symbol);
			if (auto [iter, inserted] = rules.emplace(nonterm, std::move(rule));
				not inserted) {
				auto & rule = iter->second;
				rule.emplace_back(rule_sep);
				for (const auto & token : rule) rule.emplace_back(token);
			}

			return true;
		} else if (isupper(symbol)) {
			auto nonterm = this->get_nonterminal(symbol);
			rules.emplace(nonterm, std::move(rule));
			return true;
		}
		return false;
	}

	int add_terminal(char symbol, int term) {
		if (symbols.count(term) == 0) {
			symbols.emplace(term, symbol);
			return term;
		} else if (symbols.at(term) == symbol)
			return term;

		for (auto & entry : symbols)
			if (entry.second == symbol) return entry.first;

		return this->get_terminal(symbol);
	}

	int add_nonterminal(char symbol, int nonterm) {
		if (symbols.count(nonterm) == 0) {
			symbols.emplace(nonterm, symbol);
			return nonterm;
		} else if (symbols.at(nonterm) == symbol)
			return nonterm;

		for (auto & entry : symbols)
			if (entry.second == symbol) return entry.first;

		return this->get_nonterminal(symbol);
	}

	// Helpers to get the next available item
	int next_nonterminal() const;

	int next_terminal() const;

	char next_nonterminal_symbol() const;

   private:
	std::map<int, char>				symbols{{rule_sep, rule_sep_char}};
	std::map<int, std::vector<int>> rules{};

	friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
