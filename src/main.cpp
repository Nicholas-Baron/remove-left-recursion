#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "grammar.hpp"
#include "grammar_transform.hpp"

std::string read_file(std::istream & file) {
	std::stringstream content{};
	{
		std::string line;
		while (std::getline(file, line)) { content << line << '\n'; }
	}
	return content.str();
}

std::string read_cfg_data(int arg_count, const char ** args) {
	std::string filename;
	{
		int arg_num = 1;
		while (args[arg_num] != nullptr) {
			if (arg_num == arg_count - 1) { filename = args[arg_num]; }
			arg_num++;
		}
	}

	if (filename.empty() or filename == "-" or filename == "--") {
		return read_file(std::cin);
	} else {
		std::ifstream file{filename};
		return read_file(file);
	}
}

int main(int arg_count, const char ** args) {
	const auto data = read_cfg_data(arg_count, args);

	std::cout << "\nGrammar Read From File\n" << data << std::endl;

	auto cfg = grammar::parse_from_file(data);

	std::cout << cfg << '\n';

	std::cout << "Epsilon check\n";
	const auto nonterms = cfg.nonterminals();
	for (const auto & nonterm : nonterms) {
		std::cout << std::boolalpha << nonterm << " has epsilon? "
				  << cfg.has_empty_production(nonterm) << '\n';
	}

	std::cout << "\nCycle check" << std::endl;
	const auto cycle_path = cfg.cyclic_path();
	if (not cycle_path.empty()) {
		std::cout << "Found cycle\n";
		bool print_arrow = false;
		for (const auto & node : cycle_path) {
			if (print_arrow) {
				std::cout << " --> ";
			} else {
				print_arrow = true;
			}
			std::cout << node;
		}
		std::cout << '\n';
	} else {
		std::cout << "Could not find cycle\n";
	}

	std::cout << std::endl;

	auto cleaned = remove_left_recursion(cfg);

	if (cleaned) {
		auto result = cleaned.value();
		std::cout << result << '\n';
	} else {
		std::cout << "Could not clean the grammar.\n";
	}
}
