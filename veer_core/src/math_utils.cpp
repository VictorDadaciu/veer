#include "math_utils.h"

namespace ve::math
{
glm::vec2 swapped(const glm::vec2& a) noexcept
{
    return glm::vec2(a.y, a.x);
}
}