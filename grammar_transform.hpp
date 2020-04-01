#ifndef REMOVAL_HPP
#define REMOVAL_HPP

#include <optional>

#include "grammar.hpp"

std::optional<grammar> remove_left_recursion(const grammar & input);

#endif
