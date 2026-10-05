#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>

namespace ve::bits
{
template<std::unsigned_integral flags_t>
constexpr bool get(flags_t flags, uint8_t bit)
{
    assert(bit < 8 * sizeof(flags_t));
    return flags & (1 << bit);
}

template<std::unsigned_integral flags_t>
constexpr flags_t set(flags_t flags, uint8_t bit)
{
    assert(bit < 8 * sizeof(flags_t));
    return flags | (1 << bit);
}

template<std::unsigned_integral flags_t>
constexpr flags_t unset(flags_t flags, uint8_t bit)
{
    assert(bit < 8 * sizeof(flags_t));
    return flags & ~(1 << bit);
}

template<std::unsigned_integral flags_t=size_t>
constexpr flags_t mask(uint8_t size, uint8_t offset = 0)
{
    assert(size + offset <= 8 * sizeof(flags_t));
    return ((1 << size) - 1) << offset;
}
}