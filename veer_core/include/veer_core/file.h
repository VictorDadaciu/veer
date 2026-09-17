#pragma once

#include "error_code.h"
#include "utils.h"

#include <expected>
#include <string>

namespace ve::file
{
bool exists(const std::string&) noexcept;

bool is_file(const std::string&) noexcept;
bool is_dir(const std::string&) noexcept;
bool is_absolute(const std::string&) noexcept;
bool is_relative(const std::string&) noexcept;

std::string absolute(const std::string&) noexcept;
std::string stem(const std::string&) noexcept;
std::string filename(const std::string&) noexcept;
std::string extension(const std::string&) noexcept;

[[nodiscard]]
std::expected<byte_span, error_code> read_entire_file(const std::string&) noexcept;
}