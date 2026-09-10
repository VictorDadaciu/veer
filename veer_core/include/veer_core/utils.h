#pragma once

#include <memory>
#include <meta>

namespace ve
{
using c_string = const char*;

// these inline function should be moved to source
static constexpr size_t cache_line_size = 64;
inline constexpr size_t next_multiple_of_cache_line_size(size_t x)
{
    return (x + cache_line_size - 1) & -cache_line_size;
}

inline constexpr size_t next_power_of_2(size_t x)
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

struct alignas(cache_line_size) cache_aligned_bytes
{
    std::array<std::byte, cache_line_size> bytes{};
};

inline decltype(auto) allocate_smart_cache_aligned_bytes(size_t byte_size)
{
    return std::make_unique<cache_aligned_bytes[]>(byte_size / cache_line_size);
}

inline std::byte* allocate_cache_aligned_bytes(size_t byte_size)
{
    return reinterpret_cast<std::byte*>(new cache_aligned_bytes[byte_size / cache_line_size]);
}

struct byte_span
{
    const std::byte* data{};
    size_t size{};
};

struct offset_span
{
    size_t offset{};
    size_t size{};
};

inline consteval bool is_template_of(std::meta::info type_r, std::meta::info template_r)
{
    return has_template_arguments(type_r) && template_of(type_r) == template_r;
}
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
