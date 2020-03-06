#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <ios>
#include <limits>
#include <map>
#include <vector>

class grammar {
   public:
	static constexpr int  rule_sep		= 0;
	static constexpr char rule_sep_char = '|';

	static constexpr int epsilon = std::numeric_limits<char>::max() + 1;

	static grammar parse_from_file(const std::string & data);

	// Returns each rule as its own vector
	std::vector<std::vector<int>> rule_matrix(char symbol) const;

	auto nonterminal_count() const { return rules.size(); }
	auto terminal_count() const {
		return symbols.size() - (1 + nonterminal_count());
	}

	int get_nonterminal(char symbol);

	int get_terminal(char symbol);

    bool has_empty_production(char symbol) const;

    bool is_nonterminal(char symbol) const { 
        return isupper(symbol) and symbols.count(symbol) != 0; 
    }

    bool is_terminal(char symbol) const {
        return islower(symbol) and symbols.count(symbol) != 0; 
    }

    std::vector<char> nonterminals() const {
        std::vector<char> to_ret{};
        for(const auto & entry : symbols){
            if(isupper(entry.second)){
                to_ret.push_back(entry.second);
            }
        }
        return to_ret;
    }

   private:
	std::map<int, char>				symbols{{rule_sep, rule_sep_char}};
	std::map<int, std::vector<int>> rules{};

	int next_nonterminal = 1;
	int next_terminal	 = -1;

	friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
