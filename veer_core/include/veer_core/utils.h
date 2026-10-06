#pragma once

#include <cmath>
#include <memory>

namespace ve
{
using c_string = const char*;

static constexpr size_t cache_line_size = 64;
constexpr size_t next_multiple_of_cache_line_size(size_t x)
{
    return (x + cache_line_size - 1) & -cache_line_size;
}

constexpr size_t next_power_of_2(size_t x)
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return ++x;
}

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

struct alignas(cache_line_size) cache_aligned_bytes
{
    std::array<std::byte, cache_line_size> bytes{};
};

std::byte* allocate_cache_aligned_bytes(size_t);

inline decltype(auto) allocate_smart_cache_aligned_bytes(size_t byte_size)
{
    return std::make_unique<cache_aligned_bytes[]>(byte_size / cache_line_size);
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

#define VEER_STRONG_TYPEDEF(underlying, name)                              \
class name final                                                           \
{                                                                          \
private:                                                                   \
    underlying value;                                                      \
public:                                                                    \
    inline name() = default;                                               \
    inline name(const name &x) = default;                                  \
    inline name(name &&x) = default;                                       \
    inline name &operator=(const name &rhs) = default;                     \
    inline name &operator=(name &&rhs) = default;                          \
    inline constexpr name(underlying x) noexcept : value{x} {}             \
    inline constexpr name &operator=(underlying rhs) noexcept              \
    {                                                                      \
        value = rhs;                                                       \
        return *this;                                                      \
    }                                                                      \
    inline constexpr operator const underlying &() const noexcept          \
    {                                                                      \
        return value;                                                      \
    }                                                                      \
    inline constexpr operator underlying &() noexcept { return value; }    \
    inline constexpr decltype(auto) operator==(const name &rhs) noexcept   \
    {                                                                      \
        return value == rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator!=(const name &rhs) noexcept   \
    {                                                                      \
        return value != rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator<(const name &rhs) noexcept    \
    {                                                                      \
        return value < rhs.value;                                          \
    }                                                                      \
    inline constexpr decltype(auto) operator>(const name &rhs) noexcept    \
    {                                                                      \
        return value > rhs.value;                                          \
    }                                                                      \
    inline constexpr decltype(auto) operator<=(const name &rhs) noexcept   \
    {                                                                      \
        return value <= rhs.value;                                         \
    }                                                                      \
    inline constexpr decltype(auto) operator>=(const name &rhs) noexcept   \
    {                                                                      \
        return value >= rhs.value;                                         \
    }                                                                      \
}

#ifndef VEER_KEEP_PREFIX
#define DECLARE_NO_COPY_NO_MOVE VEER_DECLARE_NO_COPY_NO_MOVE
#define DECLARE_NO_COPY         VEER_DECLARE_NO_COPY

#define STRONG_TYPEDEF          VEER_STRONG_TYPEDEF
#endif
