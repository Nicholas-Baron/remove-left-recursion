#include "grammar.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <set>

grammar grammar::parse_from_file(const std::string & data) {
	grammar to_ret{};
	auto	iter		   = data.begin();
	int		nonterm		   = 0;
	char	nonterm_symbol = ' ';

	const auto consume_whitespace = [&iter, &data]() {
		while (iter != data.end() and isspace(*iter) and *iter != '\n') iter++;
	};

	std::vector<int> rule_list{};
	while (iter != data.end()) {
		// Remove initial whitespace
		consume_whitespace();

		// Read initial symbol
		if (isupper(*iter) and to_ret.symbols.count(*iter) == 0) {
			nonterm		   = to_ret.get_nonterminal(*iter);
			nonterm_symbol = *iter;
			iter++;
		} else {
			std::cerr
				<< "Cannot use " << *iter
				<< " as a nonterminal\nNonterminals must be capitalized\n";
		}

		// remove whitespace between symbol and hyphen
		consume_whitespace();

		// consume all hyphens
		bool ate_hyphen = false;
		while (iter != data.end() and *iter == '-') {
			iter++;
			ate_hyphen = true;
		}
		if (not ate_hyphen) {
			std::cerr << "Expected some hyphens after nonterminal "
					  << nonterm_symbol << '\n';
		}

		// remove whitespace
		consume_whitespace();

		// Go to the end of the line
		while (iter != data.end() and *iter != '\n') {
			if (not isspace(*iter)) {
				if (*iter == rule_sep_char) {
					rule_list.push_back(rule_sep);
				} else if (isupper(*iter)) {
					rule_list.push_back(to_ret.get_nonterminal(*iter));
				} else {
					// lower
					rule_list.push_back(to_ret.get_terminal(*iter));
				}
			}

			iter++;
		}

		to_ret.rules.emplace(nonterm, std::move(rule_list));
		rule_list = {};
		iter++;
	}

	return to_ret;
}

std::vector<std::vector<int>> grammar::rule_matrix(int nonterminal) const {
	std::vector<std::vector<int>> to_ret{{}};

	if (rules.count(nonterminal) != 0) {
		const auto & all_rules = rules.at(nonterminal);

		to_ret.reserve(std::count(all_rules.begin(), all_rules.end(), 0));

		for (const auto & symbol : all_rules) {
			if (symbol == rule_sep) {
				to_ret.emplace_back();
			} else {
				to_ret.back().push_back(symbol);
			}
		}
	}

	return to_ret;
}

int grammar::get_nonterminal(char symbol) {
	const auto iter = std::find_if(
		symbols.begin(), symbols.end(),
		[symbol](const auto & item) { return item.second == symbol; });
	if (iter != symbols.end()) {
		return iter->first;
	} else {
		const auto to_ret = next_nonterminal;
		symbols[to_ret]	  = symbol;
		// Rules are added in the parser / manually
		next_nonterminal++;
		return to_ret;
	}
}

int grammar::get_terminal(char symbol) {
	const auto iter = std::find_if(
		symbols.begin(), symbols.end(),
		[symbol](const auto & item) { return item.second == symbol; });
	if (iter != symbols.end()) {
		return iter->first;
	} else {
		const auto to_ret = next_terminal;
		symbols[to_ret]	  = symbol;
		next_terminal--;
		return to_ret;
	}
}

bool grammar::has_empty_production(int nonterminal) const {
	if (nonterminal <= 0) return false;

	const auto & rule_list = rules.at(nonterminal);

	// An inital separator = empty production
	bool last_was_sep = true;
	for (const auto & sym : rule_list) {
		if (sym == rule_sep and last_was_sep)
			return true;  // Pipes surrounding nothing = empty production
		else
			last_was_sep = sym == rule_sep;
	}

	// Separator at the end = empty production
	return last_was_sep;
}

std::vector<int> grammar::cyclic_path() const {
	// Path so far
	std::vector<std::pair<size_t /*symbol*/, size_t /*option*/>> path{};

	size_t start_symbol = 1;

	while (start_symbol < nonterminal_count()) {
		// Pick start point
		if (path.empty()) {
			path.emplace_back(start_symbol, -1);
			start_symbol++;
		}

		// Read the options of the path and which option to chose
		const auto & current_symbol = path.back().first;
		const auto & options		= rules.at(current_symbol);
		auto		 rule_used		= path.back().second + 1;

		const auto old_path_length = path.size();
		while (path.size() == old_path_length) {
			if (rule_used < options.size() and options.at(rule_used) > 0
				and static_cast<size_t>(options.at(rule_used)) != current_symbol
				and (rule_used == 0 or options.at(rule_used - 1) == rule_sep)) {
				// found a valid rule to use
				path.back().second = rule_used;
				path.emplace_back(options.at(rule_used), -1);
			} else {
				// move to the next valid rule
				while (rule_used < options.size()
					   and options.at(rule_used) != rule_sep)
					rule_used++;
				rule_used++;
				if (rule_used >= options.size()) {
					// used all options -> go back
					path.pop_back();
				}
			}
		}

		// Check the path for repeats
		std::set<int> seen_nonterminals{};
		for (const auto & entry : path)
			if (seen_nonterminals.count(entry.first) != 0) {
				std::vector<int> to_ret(path.size(), -1);
				std::transform(path.begin(), path.end(), to_ret.begin(),
							   [](const auto & entry) { return entry.first; });
				return to_ret;
			} else
				seen_nonterminals.insert(entry.first);
	}

	return {};
}
bool grammar::using_symbol(char symbol) const {
	return std::find_if(
			   symbols.begin(), symbols.end(),
			   [symbol](const auto & entry) { return entry.second == symbol; })
		   != symbols.end();
}

std::ostream & operator<<(std::ostream & lhs, const grammar & rhs) {
	static constexpr auto arrow	 = " --> ";
	static const auto	  column = std::setw(2);

	lhs << "Symbol mapping (Negative = terminal):\n";
	for (const auto & entry : rhs.symbols) {
		lhs << column << entry.first << arrow << column << entry.second << '\n';
	}

	lhs << "Rules:\n";
	for (const auto & entry : rhs.rules) {
		lhs << column << entry.first << arrow;
		for (const auto & symbol : entry.second) {
			if (symbol == grammar::rule_sep)
				lhs << ' ' << grammar::rule_sep_char << ' ';
			else
				lhs << column << symbol << ' ';
		}
		lhs << '\n';
	}

	return lhs;
}
