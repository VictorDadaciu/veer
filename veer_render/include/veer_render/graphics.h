#pragma once

#include "window.h"

#include <veer_core/error_code.h>
#include <glm/glm.hpp>

namespace ve::gfx
{
[[nodiscard]] error_code init();
[[nodiscard]] error_code draw(window&);
void destroy();
}