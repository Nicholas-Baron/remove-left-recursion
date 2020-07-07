#ifndef STRONG_TYPES_HPP
#define STRONG_TYPES_HPP

#include <iosfwd>

// This file defines a minimal strong type system. This prevents semantically
// different values with the same type from being assigned to each other.

template<typename rep_t, typename tag_t>
class strong_t final {
   public:
    constexpr strong_t() : internal_rep{} {}
    [[maybe_unused]] explicit constexpr strong_t(rep_t rep) : internal_rep{rep} {}

    explicit constexpr operator rep_t() const { return internal_rep; }

   private:
    rep_t internal_rep;

    // Using the hidden friends idiom
    // to reduce template weight
    // and get template variables for free

    using this_t = strong_t<rep_t, tag_t>;

    // Both sides are this_t

    friend bool operator==(const this_t & lhs, const this_t & rhs) {
        return static_cast<rep_t>(lhs) == static_cast<rep_t>(rhs);
    }

    friend bool operator!=(const this_t & lhs, const this_t & rhs) {
        return static_cast<rep_t>(lhs) != static_cast<rep_t>(rhs);
    }

    friend bool operator<(const this_t & lhs, const this_t & rhs) {
        return static_cast<rep_t>(lhs) < static_cast<rep_t>(rhs);
    }

    friend bool operator<=(const this_t & lhs, const this_t & rhs) {
        return lhs < rhs or lhs == rhs;
    }

    // LHS is this_t, RHS is rep_t

    friend this_t operator+(const this_t & lhs, const rep_t & rhs) {
        return this_t(lhs.internal_rep + rhs);
    }

    friend this_t operator-(const this_t & lhs, const rep_t & rhs) {
        return this_t{lhs.internal_rep - rhs};
    }

    friend bool operator==(const this_t & lhs, const rep_t & rhs) {
        return static_cast<rep_t>(lhs) == rhs;
    }

    friend bool operator<(const this_t & lhs, const rep_t & rhs) {
        return static_cast<rep_t>(lhs) < rhs;
    }

    friend bool operator>(const this_t & lhs, const rep_t & rhs) {
        return static_cast<rep_t>(lhs) > rhs;
    }

    friend bool operator<=(const this_t & lhs, const rep_t & rhs) {
        return lhs < rhs or lhs == rhs;
    }

    // LHS is rep_t, RHS is this_t

    friend bool operator==(const rep_t & lhs, const this_t & rhs) {
        return lhs == static_cast<rep_t>(rhs);
    }

    friend std::ostream & operator<<(std::ostream & lhs, const this_t & rhs) {
        return lhs << static_cast<rep_t>(rhs);
    }
};

static_assert(sizeof(strong_t<int, struct tag>) == sizeof(int),
              "Strong types are not the size of their internal representation");

#endif
