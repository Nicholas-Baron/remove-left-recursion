#ifndef REMOVAL_HPP
#define REMOVAL_HPP

#include <optional>

#include "grammar.hpp"

std::optional<grammar> remove_left_recursion(const grammar & input);

grammar make_proper_form(const grammar & input);

grammar remove_epsilon(const grammar & input);
grammar remove_cycles(const grammar & input);
grammar remove_unreachables(const grammar & input);

#endif
