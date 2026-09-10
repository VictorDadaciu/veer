#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <expected>
#include <filesystem>

namespace ve::utils
{
[[nodiscard]] std::expected<byte_span, error_code> load(const std::filesystem::path&);
}