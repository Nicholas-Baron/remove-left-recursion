#include "grammar.hpp"

#include <algorithm>
#include <cctype>

grammar grammar::parse_from_file(const std::string& data) {

    grammar to_ret{};

    auto iter = data.begin();
    while(iter != data.end()){
        // Remove whitespace
        while(isspace(*iter) and *iter != '\n') iter++;

        
    }

    return to_ret;
}

std::vector<std::vector<int>> rules(int symbol) const {

    std::vector<std::vector<int>> to_ret{{}};

    const auto & all_rules = rules.at(symbol);

    to_ret.reserve(std::count(all_rules.begin(), all_rules.end(), 0));

    for(const auto & symbol : all_rules ){
        if(symbol == rule_sep){
            to_ret.emplace_back();
        }else{
            to_ret.back().push_back(symbol);
        }
    }

    return to_ret;
}
