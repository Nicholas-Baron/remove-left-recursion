#include "grammar.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>

using token_t  = grammar::token_t;
using symbol_t = grammar::symbol_t;
using rule_t   = std::vector<token_t>;

std::optional<grammar> grammar::parse_from_file(const std::string & data) {
    grammar     to_ret{};
    auto        iter = data.begin();
    token_t     nonterm{0};
    std::string nonterm_symbol;
    size_t      line_num = 1;

    const auto consume_whitespace = [&iter, &data] {
        while (iter != data.end() and isspace(*iter) and *iter != '\n') iter++;
    };

    const auto error = [&line_num]() -> std::ostream & {
        return std::cerr << "Line " << std::setw(2) << line_num << " : ";
    };

    const auto consume_symbol
        = [&iter, &data, &consume_whitespace]() -> std::optional<std::string> {
        if (std::string symbol; iter != data.end()) {
            consume_whitespace();

            if (*iter == '<') {
                // time to eat a whole symbol
                do {
                    symbol += *iter;
                    ++iter;

                    if (*iter == '<' or *iter == ';'
                        or *iter == grammar::rule_sep) {
                        std::cerr << "Cannot use ';', '<', or '|' in a symbol "
                                     "name\nOffending name:"
                                  << symbol << std::endl;
                        return std::optional<std::string>{};
                    }
                } while (*iter != '>');

                // Append the '>'
                iter++;
                symbol += '>';
                return std::optional{symbol};
            } else {
                auto sym = *iter;
                iter++;
                return std::optional{std::string(1, sym)};
            }
        }

        std::cerr << "Unexpected end of file" << std::endl;
        return std::optional<std::string>{};
    };

    while (iter != data.end()) {
        // Remove initial whitespace
        consume_whitespace();

        // Read initial symbol
        if (auto symbol = consume_symbol();
            symbol
            and (isupper(symbol.value().front())
                 or symbol.value().front() == '<')) {
            nonterm_symbol = symbol.value();
            nonterm        = to_ret.get_nonterminal(symbol_t{nonterm_symbol});
            std::cout << "Using token " << nonterm << " for nonterminal "
                      << nonterm_symbol << std::endl;
        } else {
            error() << "Cannot use " << *iter
                    << " as a nonterminal\nNonterminals must either be "
                       "capitalized or surrounded with <>\n";
            return std::optional<grammar>{};
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
            error() << "Expected some hyphens after nonterminal "
                    << nonterm_symbol << '\n';
        }

        // remove whitespace
        consume_whitespace();

        rule_t rule_list{};

        // Go to the end of the line
        // consuming the rest of the line as the rule
        while (iter != data.end() and *iter != '\n') {
            if (auto sym = consume_symbol(); sym) {
                if (auto symbol = sym.value(); symbol == rule_sep_char)
                    rule_list.push_back(rule_sep);
                else if (symbol_t temp_sym{symbol};
                         isupper(symbol.front()) or symbol.front() == '<')
                    rule_list.push_back(to_ret.get_nonterminal(temp_sym));
                else
                    rule_list.push_back(to_ret.get_terminal(temp_sym));
            } else {
                error() << "Could not consume next symbol in production for "
                        << nonterm << std::endl
                        << "Successfully parsed the following:";
                std::cerr << std::string{data.begin(), iter} << std::endl;
                return std::optional<grammar>{};
            }
        }

        to_ret.rules.emplace(nonterm, std::move(rule_list));
        iter++;
        line_num++;
    }

    return to_ret;
}

std::vector<std::vector<token_t>> grammar::rule_matrix(
    token_t nonterminal) const {
    std::vector<std::vector<token_t>> to_ret{{}};

    if (rules.count(nonterminal) != 0) {
        const auto & all_rules = rules.at(nonterminal);

        to_ret.reserve(std::count(all_rules.begin(), all_rules.end(), 0ul));

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

token_t grammar::get_nonterminal(symbol_t symbol) {
    const auto iter = std::find_if(
        symbols.begin(), symbols.end(),
        [symbol](const auto & item) { return item.second == symbol; });
    if (iter != symbols.end()) {
        return iter->first;
    } else {
        const auto to_ret = next_nonterminal();
        symbols[to_ret]   = symbol;
        // Rules are added in the parser / manually
        return to_ret;
    }
}

token_t grammar::get_terminal(symbol_t symbol) {
    const auto iter = std::find_if(
        symbols.begin(), symbols.end(),
        [symbol](const auto & item) { return item.second == symbol; });

    if (iter != symbols.end()) {
        return iter->first;
    } else {
        const auto to_ret = next_terminal();
        symbols[to_ret]   = symbol;
        return to_ret;
    }
}

bool grammar::has_empty_production(token_t nonterminal) const {
    if (nonterminal <= 0) return false;

    const auto & rule_list = rules.at(nonterminal);

    // An initial separator = empty production
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

std::vector<token_t> grammar::cyclic_path() const {
    // Path so far
    std::vector<std::pair<token_t /*token*/, size_t /*option*/>> path{};

    size_t start_symbol = 1;

    while (start_symbol < nonterminal_count()) {
        // Pick start point
        if (path.empty()) {
            path.emplace_back(start_symbol, -1);
            start_symbol++;
        }

        // Read the options of the path and which option to chose
        const auto & current_symbol = path.back().first;
        const auto & options        = rules.at(current_symbol);
        auto         rule_used      = path.back().second + 1;

        const auto old_path_length = path.size();
        while (path.size() == old_path_length) {
            if (rule_used < options.size()
                and options.at(rule_used) > 0
                // the rule used is pointing at a nonterminal
                and options.at(rule_used) != current_symbol
                // the rule is not generating the same symbol
                and (rule_used == 0 /*there is nothing to the left of us*/
                     or options.at(rule_used - 1) == rule_sep
                     /*the left is a rule separator*/)
                and (rule_used == options.size() - 1
                     or options.at(rule_used + 1) == rule_sep)) {
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
        std::set<token_t> seen_nonterminals{};
        for (const auto & entry : path)
            if (seen_nonterminals.count(entry.first) != 0) {
                std::vector<token_t> to_ret;
                to_ret.reserve(path.size());
                std::transform(path.begin(), path.end(),
                               std::back_inserter(to_ret),
                               [](const auto & entry) { return entry.first; });
                return to_ret;
            } else
                seen_nonterminals.insert(entry.first);
    }

    return {};
}
bool grammar::using_symbol(symbol_t symbol) const {
    return std::find_if(
               symbols.begin(), symbols.end(),
               [symbol](const auto & entry) { return entry.second == symbol; })
           != symbols.end();
}
bool grammar::is_nonterminal_symbol(symbol_t symbol) const {
    if (auto iter = std::find_if(
            symbols.begin(), symbols.end(),
            [&symbol](auto & entry) { return entry.second == symbol; });
        iter != symbols.end())
        return this->rules.count(iter->first) == 1
               and this->using_symbol(symbol);
    else
        return false;
}

bool grammar::is_terminal_symbol(symbol_t symbol) const {
    if (auto iter = std::find_if(
            symbols.begin(), symbols.end(),
            [&symbol](auto & entry) { return entry.second == symbol; });
        iter != symbols.end())
        return this->rules.count(iter->first) == 0
               and this->using_symbol(symbol);
    else
        return false;
}

std::ostream & operator<<(std::ostream & lhs, const grammar & rhs) {
    static constexpr auto arrow  = " --> ";
    static const auto     column = std::setw(2);

    lhs << "Symbol mapping (Negative = terminal):" << std::endl;
    for (const auto & entry : rhs.symbols) {
        lhs << column << entry.first << arrow << column << entry.second << '\n';
    }

    lhs << "Rules:" << std::endl;
    for (const auto & entry : rhs.rules) {
        lhs << column << entry.first << arrow;
        for (const auto & symbol : entry.second) {
            if (symbol == grammar::rule_sep)
                lhs << ' ' << grammar::rule_sep_char << ' ';
            else
                lhs << column << symbol << ' ';
        }
        lhs << std::endl;
    }

    lhs << "Rules Prettified:" << std::endl;
    for (const auto & entry : rhs.rules) {
        lhs << column << rhs.symbols.at(entry.first) << arrow;
        for (const auto & tok : entry.second) {
            if (tok == grammar::rule_sep)
                lhs << ' ' << grammar::rule_sep_char << ' ';
            else
                lhs << column << rhs.symbols.at(tok) << ' ';
        }
        lhs << std::endl;
    }

    return lhs << std::endl;
}

token_t grammar::next_nonterminal() const {
    return std::max_element(
               symbols.begin(), symbols.end(),
               [](auto & lhs, auto & rhs) { return lhs.first < rhs.first; })
               ->first
           + 1;
}

token_t grammar::next_terminal() const {
    return std::min_element(
               symbols.begin(), symbols.end(),
               [](auto & lhs, auto & rhs) { return lhs.first < rhs.first; })
               ->first
           - 1;
}

symbol_t grammar::next_nonterminal_symbol() const {
    const auto symbols = this->symbol_list();

    return std::accumulate(
        symbols.cbegin(), symbols.cend(), symbol_t{},
        [](const auto & to_ret, const auto & sym) -> symbol_t {
            auto symbol = static_cast<std::string>(sym);
            if (isupper(symbol.front()) and to_ret <= sym) {
                symbol[0] += 1;
                return symbol_t{symbol};
            } else
                return to_ret;
        });
}
