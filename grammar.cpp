#include "grammar.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

grammar grammar::parse_from_file(const std::string & data) {
	grammar to_ret{};
	auto	iter	= data.begin();
	char	nonterm = 0;

	const auto consume_whitespace = [&iter, &data]() {
		while (iter != data.end() and isspace(*iter) and *iter != '\n') iter++;
	};

	std::vector<int> rule_list{};
	while (iter != data.end()) {
		// Remove initial whitespace
		consume_whitespace();

		// Read initial symbol
		if (isupper(*iter) and to_ret.symbols.count(*iter) == 0) {
			nonterm = *iter;
			iter++;
		}

		// remove whitespace between symbol and hyphen
		consume_whitespace();

		// consume all hyphens
		while (iter != data.end() and *iter == '-') { iter++; }

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

		to_ret.rules.emplace(to_ret.get_nonterminal(nonterm), std::move(rule_list));
		rule_list = {};
		iter++;
	}

	return to_ret;
}

std::vector<std::vector<int>> grammar::rule_matrix(char symbol) const {
	std::vector<std::vector<int>> to_ret{{}};

	const auto & all_rules = rules.at(symbols.at(symbol));

	to_ret.reserve(std::count(all_rules.begin(), all_rules.end(), 0));

	for (const auto & symbol : all_rules) {
		if (symbol == rule_sep) {
			to_ret.emplace_back();
		} else {
			to_ret.back().push_back(symbol);
		}
	}

	return to_ret;
}

int grammar::get_nonterminal(char symbol) {
	auto iter = std::find_if(
		symbols.begin(), symbols.end(),
		[&](const auto & item) -> bool { return item.second == symbol; });
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
	auto iter = std::find_if(
		symbols.begin(), symbols.end(),
		[&](const auto & item) -> bool { return item.second == symbol; });
	if (iter != symbols.end()) {
		return iter->first;
	} else {
		const auto to_ret = next_terminal;
		symbols[to_ret]	  = symbol;
		next_terminal--;
		return to_ret;
	}
}

bool grammar::has_empty_production(char symbol) const {
    if(this->is_terminal(symbol)) return false;

    const auto & rule_list = rules.at(symbol);
    bool last_was_sep = true;
    for(const auto & sym : rule_list){
        if(sym == rule_sep and last_was_sep) return true;
        else if(sym == rule_sep and not last_was_sep) last_was_sep = true;
        else last_was_sep = false;
    }

    return false;
}

std::ostream & operator<<(std::ostream & lhs, const grammar & rhs) {

    static constexpr auto arrow = " --> ";
    static const auto column = std::setw(2);

    lhs << "Symbol mapping (Negative = terminal):\n";
    for(const auto& entry : rhs.symbols){
        lhs << column << entry.first << arrow << column << entry.second << '\n';
    }

    lhs << "Rules:\n";
    for( const auto& entry : rhs.rules ){
        lhs << column << entry.first << arrow;
        for(const auto & symbol : entry.second){
            if(symbol == grammar::rule_sep) lhs << ' ' << grammar::rule_sep_char << ' ';
            else lhs << column << symbol << ' ';
        }
        lhs << '\n';
    }

	return lhs;
}
