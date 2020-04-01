
#include "grammar_transform.hpp"

#include <algorithm>
#include <iostream>

using token_t = grammar::token_t;

std::optional<grammar> remove_left_recursion(const grammar & input) {
	// Preconditions: input has no empty productions and no cycles

	if (input.has_any_empty_production()) {
		std::cerr << "Input grammar has an empty production.\n";
		return {};
	}

	if (auto cycle = input.cyclic_path(); cycle.size() > 0) {
		std::cerr << "Input grammar has a cycle:\n";
		bool first = true;
		for (auto entry : cycle) {
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

	auto			  output			 = grammar::empty();
	const auto		  input_nonterm_keys = input.nonterminal_keys();
	const std::vector nonterms			 = input.nonterminals();

	for (const auto entry : input.terminal_keys())
		if (output.add_terminal(entry.second, entry.first) != entry.first)
			std::cout << entry.second << " has been remapped\n";

	for (const auto entry : input.nonterminal_keys())
		if (output.add_nonterminal(entry.second, entry.first) != entry.first)
			std::cout << entry.second << " has been remapped\n";

	for (auto i = 0ul; i < nonterms.size(); i++) {
		auto	   nonterm_i	 = nonterms.at(i);
		const auto rule_matrix_i = input.rule_matrix(nonterm_i);
		const auto nonterm_i_sym = input_nonterm_keys.at(nonterm_i);
		std::vector<std::vector<token_t>> result_matrix{};

		for (const auto & rule_i : rule_matrix_i) {
			bool removed_recursion = false;

			if (i != 0)
				for (auto j = 0ul; j < i and not removed_recursion; j++) {
					const auto nonterm_j = nonterms.at(j);

					// Replace A_i -> A_j g with A_i -> d_n g where A_j -> d_n
					if (std::vector<token_t> result_rule;
						not rule_i.empty() and rule_i.front() == nonterm_j) {
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

		if (result_matrix.empty()) result_matrix = rule_matrix_i;

		std::cout << "Before recursion removal for nonterm " << nonterm_i
				  << "(sym " << nonterm_i_sym << "):\n";
		for (auto & row : result_matrix) {
			for (auto & item : row) std::cout << ' ' << item;
			std::cout << '\n';
		}

		// Remove immediate left recursion
		bool has_left_recursion = std::any_of(
			result_matrix.begin(), result_matrix.end(),
			[&nonterm_i](const auto & rule) -> bool {
				return not rule.empty() and rule.front() == nonterm_i;
			});

		if (has_left_recursion) {
			const auto new_nonterm_sym = output.next_nonterminal_symbol();
			const auto new_nonterm	   = output.next_nonterminal();

			if (auto remapped_nonterm_i = output.get_nonterminal(nonterm_i_sym);
				remapped_nonterm_i != nonterm_i) {
				std::cout << nonterm_i_sym << " has been remapped to "
						  << remapped_nonterm_i << std::endl;
				nonterm_i = remapped_nonterm_i;
			}

			std::vector<token_t> full_rule_i;
			std::vector<token_t> full_rule_new;

			for (const auto & rule : result_matrix) {
				if (rule.empty()) {
					full_rule_i.push_back(grammar::rule_sep);
				} else if (rule.front() == nonterm_i) {
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
			bool				 first = true;
			for (const auto & rule : result_matrix) {
				if (first) {
					first = false;
				} else {
					full_rule.push_back(grammar::rule_sep);
				}
				for (auto sym : rule) full_rule.push_back(sym);
			}

			output.add_rule(nonterm_i_sym, std::move(full_rule));
		}
	}

	return std::optional{output};
}
