#pragma once

#include "mesh.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <concepts>
#include <expected>
#include <filesystem>
#include <meta>
#include <limits>
#include <variant>

namespace ve
{
enum class asset_type : uint8_t
{
    mesh,
    image,
    unknown,
};

struct asset_load_metadata
{
    const std::string name{};
    const std::filesystem::path path{};
    size_t index{};
    asset_type type = asset_type::unknown;
};

using asset_load_return_t = std::expected<std::vector<asset_load_metadata>, error_code>;
}

namespace ve::assets
{
[[nodiscard]]
asset_load_return_t load(const std::filesystem::path&);

ve::mesh& mesh(const size_t&);

void unload_all();
}