#pragma once

#include <cmath>
#include <memory>

namespace ve
{
using c_string = const char*;

template<int exponent, std::floating_point underlying_t=float>
struct tolerance_t
{
    static constexpr underlying_t value = std::pow(static_cast<underlying_t>(10), static_cast<underlying_t>(exponent));
};
template<int exponent, std::floating_point underlying_t=float>
static constexpr underlying_t tolerance = tolerance_t<exponent, underlying_t>::value;

template<std::floating_point f1_t, std::floating_point f2_t, std::floating_point bigger_t = std::conditional_t<(sizeof(f1_t) >= sizeof(f2_t)), f1_t, f2_t>>
inline constexpr bool fuzzy_eq(f1_t a, f2_t b, bigger_t tol = tolerance<-6, bigger_t>)
{
    return std::abs(a - b) <= tol;
}

struct byte_span
{
    void invalidate()
    {
        data = nullptr;
        size = 0zu;
    }

    std::byte* data{};
    size_t size{};
};

struct offset_span
{
    size_t offset{};
    size_t size{};
};
}

#define VEER_DECLARE_NO_COPY_NO_MOVE(x) \
x() = default;                          \
x(const x&) = delete;                   \
x(x&&) = delete;                        \
x& operator=(const x&) = delete;        \
x& operator=(const x&&) = delete;

#define VEER_DECLARE_NO_COPY(x) \
x() = default;                  \
x(const x&) = delete;           \
x& operator=(const x&) = delete;

#define _VEER_SINGLETON(x)      \
inline static x& get() noexcept \
{                               \
    static x instance;          \
    return instance;            \
}

#ifndef VEER_DONT_STRIP_PREFIX
#define DECLARE_NO_COPY_NO_MOVE VEER_DECLARE_NO_COPY_NO_MOVE
#define DECLARE_NO_COPY         VEER_DECLARE_NO_COPY
#endif
