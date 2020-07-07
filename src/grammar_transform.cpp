
#include "grammar_transform.hpp"

#include <algorithm>
#include <iostream>
#include <set>

using token_t = grammar::token_t;

// A template to help with checking if a container contains an item
template<typename T, typename Iter>
bool contains(Iter begin, Iter end, T item) {
    return std::find(begin, end, item) != end;
}

std::optional<grammar> remove_left_recursion(const grammar & input) {
    // Preconditions: input has no empty productions and no cycles

    if (input.has_any_empty_production()) {
        std::cerr << "Input grammar has an empty production.\n";
        return {};
    }

    if (const auto cycle = input.cyclic_path(); !cycle.empty()) {
        std::cerr << "Input grammar has a cycle:\n";
        bool first = true;
        for (const auto & entry : cycle) {
            if (first) {
                first = false;
            } else {
                std::cerr << " --> ";
            }
            std::cerr << entry;
        }

        std::cerr << '\n';
        return {};
    }

    auto              output             = grammar::copy_terminals_from(input);
    const auto        input_nonterm_keys = input.nonterminal_keys();
    const std::vector nonterms           = input.nonterminals();

    for (const auto & entry : input.nonterminal_keys())
        if (output.add_nonterminal(entry.second, entry.first) != entry.first)
            std::cout << entry.second << " has been remapped\n";

    for (auto i = 0ul; i < nonterms.size(); i++) {
        auto         nonterm_i     = nonterms.at(i);
        const auto   rule_matrix_i = input.rule_matrix(nonterm_i);
        const auto & nonterm_i_sym = input_nonterm_keys.at(nonterm_i);
        std::vector<std::vector<token_t>> result_matrix{};

        for (const auto & rule_i : rule_matrix_i) {
            bool removed_recursion = false;

            if (i != 0)
                for (auto j = 0ul; j < i and not removed_recursion; j++) {
                    const auto nonterm_j = nonterms.at(j);

                    // Replace A_i -> A_j g with A_i -> d_n g where A_j -> d_n
                    // As there are no epsilon rules, front() can be used with
                    // impunity.
                    if (std::vector<token_t> result_rule;
                        rule_i.front() == nonterm_j) {
                        const auto rule_matrix_j
                            = output.rule_matrix(nonterm_j);
                        for (const auto & rule_j : rule_matrix_j) {
                            std::copy(rule_j.cbegin(), rule_j.cend(),
                                      std::back_inserter(result_rule));

                            bool first = true;
                            for (const auto & item : rule_i)
                                if (first)
                                    first = false;
                                else
                                    result_rule.push_back(item);

                            result_matrix.push_back(std::move(result_rule));
                            result_rule = {};
                        }
                        removed_recursion = true;
                    }
                }

            if (not removed_recursion) result_matrix.push_back(rule_i);
        }

        // At this point, the rule matrix is definitely full of rules.
        // Either a rule had to have left recursion removed
        // or it was copied wholesale from the input grammar.

        std::cout << "Before immediate recursion removal for nonterm "
                  << nonterm_i << "(sym " << nonterm_i_sym << "):\n";
        for (auto & row : result_matrix) {
            for (auto & item : row) std::cout << ' ' << item;
            std::cout << '\n';
        }

        // Remove immediate left recursion
        const bool has_left_recursion = std::any_of(
            result_matrix.begin(), result_matrix.end(),
            [&nonterm_i](const auto & rule) {
                return not rule.empty() and rule.front() == nonterm_i;
            });

        if (has_left_recursion) {
            const auto new_nonterm_sym = output.next_nonterminal_symbol();
            const auto new_nonterm     = output.next_nonterminal();

            if (auto remapped_nonterm_i = output.get_nonterminal(nonterm_i_sym);
                remapped_nonterm_i != nonterm_i) {
                std::cout << nonterm_i_sym << " has been remapped to "
                          << remapped_nonterm_i << std::endl;
                nonterm_i = remapped_nonterm_i;
            }

            std::vector<token_t> full_rule_i;
            std::vector<token_t> full_rule_new;

            for (const auto & rule : result_matrix) {
                if (rule.empty())
                    full_rule_i.push_back(grammar::rule_sep);
                else if (rule.front() == nonterm_i) {
                    bool first = true;
                    full_rule_new.push_back(grammar::rule_sep);
                    // skip the first element, copy the left recursive rule into
                    // the new symbol
                    for (auto token : rule)
                        if (not first)
                            full_rule_new.push_back(token);
                        else
                            first = false;

                    full_rule_new.push_back(new_nonterm);
                } else {
                    if (not full_rule_i.empty()
                        and full_rule_i.back() != grammar::rule_sep)
                        full_rule_i.push_back(grammar::rule_sep);

                    for (auto token : rule) full_rule_i.push_back(token);

                    full_rule_i.push_back(new_nonterm);
                }
            }

            output.add_rule(nonterm_i_sym, std::move(full_rule_i));
            output.add_rule(new_nonterm_sym, std::move(full_rule_new));

        } else {
            std::vector<token_t> full_rule;
            bool                 first = true;
            for (const auto & rule : result_matrix) {
                if (first)
                    first = false;
                else
                    full_rule.push_back(grammar::rule_sep);

                for (auto sym : rule) full_rule.push_back(sym);
            }

            output.add_rule(nonterm_i_sym, std::move(full_rule));
        }
    }

    return std::optional{output};
}

grammar make_proper_form(const grammar & input) {
    // TODO: remove unproductive symbols?
    return remove_unreachables(remove_unit_productions(remove_epsilon(input)));
}

grammar remove_epsilon(const grammar & input) {
    if (not input.has_any_empty_production()) return input;

    auto              output             = grammar::copy_terminals_from(input);
    const auto        input_nonterm_keys = input.nonterminal_keys();
    const std::vector nonterms           = input.nonterminals();

    // If the first symbol has epsilon, check if it is used anywhere
    if (input.has_empty_production(nonterms.front())) {
        if (input.in_some_production(nonterms.front())) {
            // Since the initial symbol is used somewhere in the grammar
            // and it can generate empty,
            // the grammmar must be augmented.

            // This is done in the form B -> A | empty ; A -> a_1 | a_2
            // where the original rule was A -> empty | a_1 | a_2

            const auto& initial_sym = input_nonterm_keys.at(nonterms.front());

            auto initial
                = output.add_nonterminal(initial_sym, nonterms.front());

            // The new symbol added from the augmentation
            auto true_initial_sym = output.next_nonterminal_symbol();
            output.add_nonterminal(true_initial_sym, output.next_nonterminal());

            output.add_rule(true_initial_sym,
                            std::vector<token_t>{initial, grammar::rule_sep});

            std::cout << "Grammar has been augmented\n";
        }
    }

    // For every rule with a nonterminal that can be epsilon in it,
    // that rule is duplicated and the copy has the nonterminal removed
    std::set<token_t> to_remove;
    for (const auto & nonterm : nonterms)
        if (input.has_empty_production(nonterm)) to_remove.emplace(nonterm);

    for (const auto & nonterm : nonterms) {
        auto rule_matrix = input.rule_matrix(nonterm);
        rule_matrix.reserve(rule_matrix.size() + to_remove.size());

        for (auto & rule : rule_matrix)
            for (const auto & removing : to_remove)
                if (const auto loc
                    = std::find(rule.begin(), rule.end(), removing);
                    loc != rule.end()) {
                    rule_matrix.emplace_back(rule);
                    rule.erase(loc);
                }

        std::vector<token_t> final_rule{};
        bool                 first = true;
        for (const auto & rule : rule_matrix) {
            if (rule.empty()) continue;

            if (first)
                first = false;
            else
                final_rule.push_back(grammar::rule_sep);

            for (const auto & tok : rule) final_rule.push_back(tok);
        }

        output.add_rule(input_nonterm_keys.at(nonterm), std::move(final_rule));
    }

    std::cout << "Result:\n";
    std::cout << output << std::endl;
    return output;
}

grammar remove_unit_productions(grammar input) {
    while (input.has_any_cycle()) {
        auto              output = grammar::copy_terminals_from(input);
        const auto        input_nonterm_keys = input.nonterminal_keys();
        const std::vector nonterms           = input.nonterminals();

        for (const auto & nonterm : nonterms) {
            auto rule_matrix = input.rule_matrix(nonterm);
            auto new_rules   = decltype(rule_matrix){};

            for (auto iter = rule_matrix.begin(); iter != rule_matrix.end();
                 ++iter) {
                auto dest_nonterm = iter->front();
                if (iter->size() == 1
                    and contains(nonterms.begin(), nonterms.end(),
                                 dest_nonterm)) {
                    if (auto last_iter = iter - 1;
                        iter == rule_matrix.begin()) {
                        iter = rule_matrix.erase(iter);
                    } else {
                        rule_matrix.erase(iter);
                        iter = last_iter;
                    }
                    for (const auto & dest_rule :
                         input.rule_matrix(dest_nonterm))
                        new_rules.emplace_back(dest_rule);
                }
            }

            std::vector<token_t> final_rule{};
            bool                 first = true;
            for (const auto & rule : rule_matrix) {
                if (rule.empty()) continue;

                if (first)
                    first = false;
                else
                    final_rule.push_back(grammar::rule_sep);

                for (const auto & tok : rule) final_rule.push_back(tok);
            }

            for (const auto & rule : new_rules) {
                if (not contains(rule_matrix.begin(), rule_matrix.end(), rule)
                    and not(rule.size() == 1 and rule.front() == nonterm)) {
                    // If the rule was not already added
                    // and it is not a rule of the form "A -> A"
                    // , add it
                    final_rule.push_back(grammar::rule_sep);
                    for (auto tok : rule) { final_rule.push_back(tok); }
                }
            }

            output.add_rule(input_nonterm_keys.at(nonterm),
                            std::move(final_rule));
        }

        input = std::move(output);
    }

    return input;
}

grammar remove_unreachables(const grammar & input) {
    auto              output             = grammar::copy_terminals_from(input);
    const auto        input_nonterm_keys = input.nonterminal_keys();
    const std::vector nonterms           = input.nonterminals();

    // Save all reachable nonterminals
    std::vector reachable{nonterms.front()};
    for (size_t reachable_count = 0; reachable_count < reachable.size();
         ++reachable_count) {
        for (const auto & rule :
             input.rule_matrix(reachable.at(reachable_count))) {
            for (auto token : rule) {
                // If a token is reachable, a nonterminal, and is not already
                // reachable, add it to the list of reachables
                if (contains(nonterms.begin(), nonterms.end(), token)
                    and not contains(reachable.begin(), reachable.end(),
                                     token)) {
                    reachable.push_back(token);
                }
            }
        }
    }

    for (auto nonterm : reachable) {
        std::vector<token_t> final_rule{};
        bool                 first = true;
        for (const auto & rule : input.rule_matrix(nonterm)) {
            if (first)
                first = false;
            else
                final_rule.push_back(grammar::rule_sep);

            for (const auto & tok : rule) final_rule.push_back(tok);
        }
        output.add_rule(input_nonterm_keys.at(nonterm), std::move(final_rule));
    }

    return output;
}
