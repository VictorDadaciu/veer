#pragma once

#include "timing.h"

#include <veer_core/utils.h>

namespace ve
{
struct game_clock
{
    void advance_frame();

    time_point frame_start = time::now();
    float dt = 1.f / 144.f;
    float real_dt = 1.f / 144.f;
    float warp = 1.f;

    _VEER_SINGLETON(game_clock);
};
}