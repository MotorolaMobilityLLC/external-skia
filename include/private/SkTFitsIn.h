/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTFitsIn_DEFINED
#define SkTFitsIn_DEFINED

#include <cassert>
#include <limits>
#include <type_traits>

/**
 * In C++ an unsigned to signed cast where the source value cannot be represented in the destination
 * type results in an implementation defined destination value. Unlike C, C++ does not allow a trap.
 * This makes "(S)(D)s == s" a possibly useful test. However, there are two cases where this is
 * incorrect:
 *
 * when testing if a value of a smaller signed type can be represented in a larger unsigned type
 * (int8_t)(uint16_t)-1 == -1 => (int8_t)0xFFFF == -1 => [implementation defined] == -1
 *
 * when testing if a value of a larger unsigned type can be represented in a smaller signed type
 * (uint16_t)(int8_t)0xFFFF == 0xFFFF => (uint16_t)-1 == 0xFFFF => 0xFFFF == 0xFFFF => true.
 *
 * Consider the cases:
 *   u = unsigned, less digits
 *   U = unsigned, more digits
 *   s = signed, less digits
 *   S = signed, more digits
 *   v is the value we're considering.
 *
 * u -> U: (u)(U)v == v, trivially true
 * U -> u: (U)(u)v == v, both casts well defined, test works
 * s -> S: (s)(S)v == v, trivially true
 * S -> s: (S)(s)v == v, first cast implementation value, second cast defined, test works
 * s -> U: (s)(U)v == v, *this is bad*, the second cast results in implementation defined value
 * S -> u: (S)(u)v == v, the second cast is required to prevent promotion of rhs to unsigned
 * u -> S: (u)(S)v == v, trivially true
 * U -> s: (U)(s)v == v, *this is bad*,
 *                             first cast results in implementation defined value,
 *                             second cast is defined. However, this creates false positives
 *                             uint16_t x = 0xFFFF
 *                                (uint16_t)(int8_t)x == x
 *                             => (uint16_t)-1        == x
 *                             => 0xFFFF              == x
 *                             => true
 *
 * So for the eight cases three are trivially true, three more are valid casts, and two are special.
 * The two 'full' checks which otherwise require two comparisons are valid cast checks.
 * The two remaining checks s -> U [v >= 0] and U -> s [v <= max(s)] can be done with one op.
 */

template <typename D, typename S>
static constexpr inline bool SkTFitsIn(S src) {
    // SkTFitsIn() is used in public headers, so needs to be written targeting at most C++11.
    return assert(std::is_integral<S>::value || std::is_enum<S>::value),
           assert(std::is_integral<D>::value || std::is_enum<D>::value),

    // E.g. (int8_t)(uint8_t) int8_t(-1) == -1, but the uint8_t == 255, not -1.
    (std::is_signed<S>::value && std::is_unsigned<D>::value && sizeof(S) <= sizeof(D)) ?
        (S)0 <= src :

    // E.g. (uint8_t)(int8_t) uint8_t(255) == 255, but the int8_t == -1.
    (std::is_signed<D>::value && std::is_unsigned<S>::value && sizeof(D) <= sizeof(S)) ?
        src <= (S)std::numeric_limits<D>::max() :

    // else
        (S)(D)src == src;
}

#endif
