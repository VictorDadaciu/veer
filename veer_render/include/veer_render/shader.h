#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <expected>

namespace ve::shader
{
enum class stage : uint8_t
{
    vertex,
    fragment,
    unknown
};

[[nodiscard]] std::expected<size_t, error_code> load(const std::string&);

void destroy_all();
}