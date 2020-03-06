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
	std::vector<std::vector<int>> rule_matrix(int symbol) const;

	auto nonterminal_count() const { return rules.size(); }
	auto terminal_count() const {
		return symbols.size() - (1 + nonterminal_count());
	}

	int nonterminal(char symbol);

	int terminal(char symbol);

   private:
	std::map<int, char>				symbols{{rule_sep, rule_sep_char}};
	std::map<int, std::vector<int>> rules{};

	int next_nonterminal = 1;
	int next_terminal	 = -1;

	friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
