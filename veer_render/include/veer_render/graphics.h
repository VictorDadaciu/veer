#pragma once

#include "vk_command.h"

#include <veer_core/error_code.h>

#include <glm/glm.hpp>

namespace ve
{
struct window;
}

namespace ve::gfx
{
[[nodiscard]] error_code init();
[[nodiscard]] std::expected<vk_command_buffer, error_code> begin_draw(window&);
[[nodiscard]] error_code end_draw(window&, vk_command_buffer&);
void destroy();
}