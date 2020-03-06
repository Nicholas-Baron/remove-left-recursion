#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "grammar.hpp"

std::string read_file(std::ifstream && file) {
	std::stringstream content{};
	{
		std::string line;
		while (std::getline(file, line)) content << line << '\n';
	}
	return content.str();
}

int main(int arg_count, const char ** args) {
	std::string filename;
	{
		int arg_num = 1;
		while (args[arg_num] != nullptr) {
			if (arg_num == arg_count - 1) filename = args[arg_num];
			arg_num++;
		}
	}

	const auto data = read_file(std::ifstream{filename});

	std::cout << data << std::flush;

	auto cfg = grammar::parse_from_file(data);

	std::cout << cfg << std::endl;
}
