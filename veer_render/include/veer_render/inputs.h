#pragma once

namespace ve::inputs
{
void process();
[[nodiscard]] bool quit_requested() noexcept; // TODO: I think this is bad
[[nodiscard]] bool pressing() noexcept; // TODO: I think this is bad
}