#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "grammar.hpp"

std::string read_file(std::istream & file) {
	std::stringstream content{};
	{
		std::string line;
		while (std::getline(file, line)) content << line << '\n';
	}
	return content.str();
}

std::string read_cfg_data(int arg_count, const char** args){
	std::string filename;
	{
		int arg_num = 1;
		while (args[arg_num] != nullptr) {
			if (arg_num == arg_count - 1) filename = args[arg_num];
			arg_num++;
		}
	}

    if(filename.empty() or filename == "-" or filename == "--"){
        return read_file(std::cin);
    }else{
        std::ifstream file{filename};
        return read_file(file);
    }

}

int main(int arg_count, const char ** args) {
	const auto data = read_cfg_data(arg_count, args);

	std::cout << "Grammar Read From File\n" << data << std::endl;

	auto cfg = grammar::parse_from_file(data);

	std::cout << cfg << std::endl;
}
