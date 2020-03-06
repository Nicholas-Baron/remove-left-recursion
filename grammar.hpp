#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <vector>
#include <map>

class grammar{
    public:
        static constexpr int rule_sep = 0;
        static constexpr char rule_sep_char = '|';

        static grammar parse_from_file(const std::string& data);

        // Returns each rule as its own vector
        std::vector<std::vector<int>> rules(int symbol) const;

        auto nonterm_count() const { return rules.size(); }
    private:
        std::map<int, char> symbols{ { rule_sep, rule_sep_char } };
        std::map<int, std::vector<int>> rules{};
};

#endif
