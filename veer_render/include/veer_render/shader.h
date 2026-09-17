#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <expected>

namespace ve::shader
{
[[nodiscard]] std::expected<size_t, error_code> load(const std::string&);

void destroy_all();
}