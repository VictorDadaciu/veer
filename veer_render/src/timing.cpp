#include "timing.h"

#include "internal/game_clock.h"

#include <veer_core/log.h>

namespace ve
{
void game_clock::advance_frame()
{
    auto& gc = game_clock::get();
    auto now = time::now();
    gc.real_dt = time::duration(gc.frame_start, now);
    gc.dt = gc.warp * gc.real_dt;
    gc.frame_start = now;
}
}

namespace ve::time
{
using namespace ve;
float dt() noexcept
{
    auto& gc = game_clock::get();
    return gc.dt;
}

float real_dt() noexcept
{
    auto& gc = game_clock::get();
    return gc.real_dt;
}

float warp() noexcept
{
    auto& gc = game_clock::get();
    return gc.warp;
}

void set_warp(float new_warp) noexcept
{
    if (!(new_warp > 0.f))
    {
        warn("Time warp must be positive, ignoring: {}", new_warp);
        return;
    }

    auto& gc = game_clock::get();
    gc.warp = new_warp;
}

const time_point& frame_start() noexcept
{
    auto& gc = game_clock::get();
    return gc.frame_start;
}
}