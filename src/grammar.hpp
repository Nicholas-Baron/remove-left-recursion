#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP

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

    static grammar                empty() { return grammar{}; }
    static std::optional<grammar> parse_from_file(const std::string & data);

    // Returns each rule as its own vector
    [[nodiscard]] std::vector<std::vector<token_t>> rule_matrix(
        token_t nonterminal) const;

    [[nodiscard]] auto nonterminal_count() const { return rules.size(); }
    [[nodiscard]] auto terminal_count() const {
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

    [[nodiscard]] bool has_any_cycle() const {
        return !this->cyclic_path().empty();
    }

    // Prints one possible cycle path
    // Empty if could not find one
    [[nodiscard]] std::vector<token_t> cyclic_path() const;

    [[nodiscard]] bool using_symbol(const symbol_t & symbol) const;

    [[nodiscard]] bool is_nonterminal_symbol(symbol_t symbol) const;

    [[nodiscard]] bool is_terminal_symbol(symbol_t symbol) const;

    [[nodiscard]] std::vector<token_t> nonterminals() const {
        std::vector<token_t> to_ret{};
        for (const auto & entry : symbols) {
            if (entry.first > 0) { to_ret.push_back(entry.first); }
        }
        return to_ret;
    }

    [[nodiscard]] std::vector<token_t> terminals() const {
        std::vector<token_t> to_ret{};
        for (const auto & entry : symbols) {
            if (entry.first < 0) { to_ret.push_back(entry.first); }
        }
        return to_ret;
    }

    [[nodiscard]] std::vector<symbol_t> symbol_list() const {
        std::vector<symbol_t> to_ret;
        to_ret.reserve(symbols.size());
        for (const auto & [_, letter] : symbols) {
            if (letter != rule_sep_char()) to_ret.emplace_back(letter);
        }
        return to_ret;
    }

    [[nodiscard]] std::map<token_t, symbol_t> nonterminal_keys() const {
        std::map<token_t, symbol_t> to_ret;
        auto                        nonterms = this->nonterminals();
        for (const auto nonterm : nonterms)
            to_ret.emplace(nonterm, symbols.at(nonterm));

        return to_ret;
    }

    [[nodiscard]] std::map<token_t, symbol_t> terminal_keys() const {
        std::map<token_t, symbol_t> to_ret;
        auto                        terms = this->terminals();
        for (const auto term : terms) to_ret.emplace(term, symbols.at(term));

        return to_ret;
    }

    // Returns true if the rule was successfully added
    bool add_rule(symbol_t symbol, std::vector<token_t> && rule) {
        if (this->is_nonterminal_symbol(symbol)) {
            const auto nonterm = get_nonterminal(symbol);
            if (auto [iter, inserted] = rules.emplace(nonterm, std::move(rule));
                not inserted) {
                auto & rule = iter->second;
                rule.emplace_back(rule_sep);
                for (const auto & token : rule) rule.emplace_back(token);
            }

            return true;
        } else if (auto first_char = static_cast<std::string>(symbol).front();
                   isupper(first_char) or first_char == '<') {
            auto nonterm = this->get_nonterminal(symbol);
            rules.emplace(nonterm, std::move(rule));
            return true;
        }
        return false;
    }

    token_t add_terminal(symbol_t symbol, token_t term) {
        if (symbols.count(term) == 0) {
            symbols.emplace(term, symbol);
            return term;
        } else if (symbols.at(term) == symbol)
            return term;

        for (auto & entry : symbols)
            if (entry.second == symbol) return entry.first;

        return this->get_terminal(symbol);
    }

    token_t add_nonterminal(symbol_t symbol, token_t nonterm) {
        if (symbols.count(nonterm) == 0) {
            symbols.emplace(nonterm, symbol);
            return nonterm;
        } else if (symbols.at(nonterm) == symbol)
            return nonterm;

        for (auto & entry : symbols)
            if (entry.second == symbol) return entry.first;

        return this->get_nonterminal(symbol);
    }

    // Helpers to get the next available item
    [[nodiscard]] token_t next_nonterminal() const;

    [[nodiscard]] token_t next_terminal() const;

    [[nodiscard]] symbol_t next_nonterminal_symbol() const;

   private:
    explicit grammar() = default;
    std::map<token_t, symbol_t> symbols{{rule_sep, rule_sep_char()}};
    std::map<token_t, std::vector<token_t>> rules{};

    friend std::ostream & operator<<(std::ostream & lhs, const grammar & rhs);
};

#endif
