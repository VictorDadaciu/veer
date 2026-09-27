#pragma once

#include <veer_core/timing.h>

namespace ve::time
{
[[nodiscard]] float dt() noexcept;
[[nodiscard]] float real_dt() noexcept;
[[nodiscard]] float warp() noexcept;
void set_warp(float) noexcept;
[[nodiscard]] const ve::time_point& frame_start() noexcept;
}