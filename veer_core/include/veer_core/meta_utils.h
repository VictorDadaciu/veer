#pragma once

#include <meta>

namespace ve
{
inline consteval bool is_template_of(std::meta::info type_r, std::meta::info template_r)
{
    return has_template_arguments(type_r) && template_of(type_r) == template_r;
}
}