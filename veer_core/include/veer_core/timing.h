#pragma once

#include <chrono>

namespace ve
{
using clock = std::chrono::steady_clock;
using time_point = clock::time_point;
}

namespace ve::time
{
[[nodiscard]] ve::time_point now() noexcept;
[[nodiscard]] float duration(const ve::time_point&, const ve::time_point&) noexcept;
[[nodiscard]] float seconds(const ve::time_point&) noexcept;
[[nodiscard]] float milliseconds(const ve::time_point&) noexcept;
[[nodiscard]] float microseconds(const ve::time_point&) noexcept;
[[nodiscard]] float nanoseconds(const ve::time_point&) noexcept;
}