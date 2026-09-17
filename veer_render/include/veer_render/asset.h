#pragma once

#include "mesh.h"
#include "texture.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <concepts>
#include <expected>
#include <meta>
#include <limits>
#include <variant>

namespace ve
{
enum class asset_type : uint8_t
{
    mesh,
    texture,
    unknown,
};

struct asset_load_metadata
{
    const std::string name{};
    size_t index = std::numeric_limits<size_t>::max();
    asset_type type = asset_type::unknown;
};

using asset_load_return_t = std::expected<std::vector<asset_load_metadata>, error_code>;
}

namespace ve::assets
{
[[nodiscard]]
asset_load_return_t load(const std::string&);

ve::mesh& mesh(const size_t&);
ve::texture& texture(const size_t&);

void unload_all();
}