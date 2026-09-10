#pragma once

#include "error_code.h"
#include "utils.h"

#include <expected>
#include <filesystem>

namespace ve::utils
{
std::expected<byte_span, error_code> read_entire_file(const std::filesystem::path&);
}