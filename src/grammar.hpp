#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

#include <algorithm>
#include <ios>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "strong_types.hpp"

// This class stores the grammar, while allowing some higher level manipulations
// on it. The nonterminals are the positive numbers, while the terminals are the
// negative numbers.
class grammar {
    // TODO: Write replace_rule
    // TODO: Sort the member functions both in the header and cpp
    // TODO: Convert to the rule_t = std::vector<token_t>

    struct token_tag {};
    struct symbol_tag {};

   public:
    // A token is the internal representation
    // of a nonterminal, terminal, or the rule separator.
    // It is exposed as part of the interface for strong typing support.
    using token_t = strong_t<int, token_tag>;

    // A symbol is the printable version
    // of a nonterminal, terminal, or the rule separator.
    // It is exposed as part of the interface for strong typing support.
    using symbol_t = strong_t<std::string, symbol_tag>;

    static constexpr token_t             rule_sep{0};
    [[nodiscard]] static inline symbol_t rule_sep_char() {
        return symbol_t{"|"};
    };

    static grammar                              empty() { return grammar{}; }
    [[nodiscard]] static std::optional<grammar> parse_from_file(
        const std::string & data);
    // Create a new grammar with the same nonterminals as the input
    [[nodiscard]] static grammar copy_terminals_from(const grammar &);

    // Returns each rule as its own vector
    [[nodiscard]] std::vector<std::vector<token_t>> rule_matrix(
        token_t nonterminal) const;

    [[nodiscard]] auto nonterminal_count() const { return rules.size(); }
    [[maybe_unused]] [[nodiscard]] auto terminal_count() const {
        return symbols.size() - (1 + nonterminal_count());
    }

    token_t get_nonterminal(const symbol_t & symbol);

    token_t get_terminal(const symbol_t & symbol);

    [[nodiscard]] bool has_empty_production(token_t nonterminal) const;

    [[nodiscard]] bool has_any_empty_production() const {
        for (const auto nonterm : this->nonterminals())
            if (this->has_empty_production(nonterm)) return true;

        return false;
    }

    [[nodiscard]] bool in_some_production(const token_t & tok) const {
        for (const auto & entry : rules)
            if (std::any_of(
                    entry.second.begin(), entry.second.end(),
                    [&tok](const auto & token) { return tok == token; }))
                return true;

        return false;
    }

    [[nodiscard]] bool has_any_cycle() const {
        return not this->cyclic_path().empty();
    }

    // Prints one possible cycle path
    // Empty if could not find one
    [[nodiscard]] std::vector<token_t> cyclic_path() const;

    [[nodiscard]] bool using_symbol(const symbol_t & symbol) const;

    [[nodiscard]] bool is_nonterminal_symbol(symbol_t symbol) const;

    [[maybe_unused]] [[nodiscard]] bool is_terminal_symbol(symbol_t symbol) const;

    [[nodiscard]] std::vector<token_t> nonterminals() const;

    [[nodiscard]] std::vector<token_t> terminals() const;

    [[nodiscard]] std::vector<symbol_t> symbol_list() const;

    [[nodiscard]] std::map<token_t, symbol_t> nonterminal_keys() const;

    [[nodiscard]] std::map<token_t, symbol_t> terminal_keys() const;

    // Returns true if the rule was successfully added
    bool add_rule(const symbol_t & symbol, std::vector<token_t> && rule);

    token_t add_terminal(const symbol_t & symbol, token_t term);

    token_t add_nonterminal(const symbol_t & symbol, token_t nonterm);

    // Helpers to get the next available item
    [[nodiscard]] token_t next_nonterminal() const;

    [[nodiscard]] token_t next_terminal() const;

    [[nodiscard]] symbol_t next_nonterminal_symbol() const;

   private:
    [[nodiscard]] explicit grammar() = default;
    std::map<token_t, symbol_t> symbols{{rule_sep, rule_sep_char()}};
    std::map<token_t, std::vector<token_t>> rules{};

    friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
