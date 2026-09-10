#pragma once

#include <veer_core/error_code.h>
#include <glm/glm.hpp>

namespace ve::gfx
{
[[nodiscard]] error_code init();
void destroy();
}