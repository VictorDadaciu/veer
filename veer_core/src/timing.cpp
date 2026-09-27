#include "timing.h"

namespace ve::time
{
using namespace ve;
time_point now() noexcept
{
    return clock::now();
}

float duration(const time_point& from, const time_point& to) noexcept
{
    return std::chrono::duration<float, std::chrono::seconds::period>(to - from).count();
}

float seconds(const time_point& tp) noexcept
{
    return std::chrono::duration<float, std::chrono::seconds::period>(tp.time_since_epoch()).count();
}

float milliseconds(const time_point& tp) noexcept
{
    return std::chrono::duration<float, std::chrono::milliseconds::period>(tp.time_since_epoch()).count();
}

float microseconds(const time_point& tp) noexcept
{
    return std::chrono::duration<float, std::chrono::microseconds::period>(tp.time_since_epoch()).count();
}

float nanoseconds(const time_point& tp) noexcept
{
    return std::chrono::duration<float, std::chrono::nanoseconds::period>(tp.time_since_epoch()).count();
}
}