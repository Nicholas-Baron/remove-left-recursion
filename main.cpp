#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

std::string read_file(std::ifstream&& file){

    std::stringstream content{};
    {
        std::string line;
        while(std::getline(file, line)) content << line << '\n';
    }
    return content.str();
}

int main(int arg_count, const char** args){
    
    std::string filename;
    {
        int arg_num = 1;
        while(args[arg_num] != nullptr){
        
            if(arg_num == arg_count - 1) filename = args[arg_num];
            arg_num++;
        }
    }

    auto data = read_file(std::ifstream{filename});

    std::cout << data << std::flush;
}
