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
   public:
	static constexpr int  rule_sep		= 0;
	static constexpr char rule_sep_char = '|';

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

	bool has_cycle() const { return this->cyclic_path().size() > 0; }

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

	void change_rules(int symbol, std::vector<int> && new_rules) {
		if (rules.count(symbol) != 0) rules.at(symbol) = std::move(new_rules);
	}

   private:
	std::map<int, char>				symbols{{rule_sep, rule_sep_char}};
	std::map<int, std::vector<int>> rules{};

	int next_nonterminal = 1;
	int next_terminal	 = -1;

	friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
