#pragma once

#include "asset.h"

#include <string>
#include <vector>

namespace ve::assets
{
struct asset_manager
{
    VEER_DECLARE_NO_COPY(asset_manager);

    error_code init() noexcept { return error_code::success; }
    ~asset_manager() = default;

    template<class asset_t>
    void unload_back(std::vector<asset_t>& asset_vec, size_t n=std::numeric_limits<size_t>::max()) noexcept
    {
        if (n >= asset_vec.size())
        {
            for (auto& asset : asset_vec)
                asset.destroy();
            asset_vec.clear();
            return;
        }
        size_t i = asset_vec.size();
        while (n-- > 0)
        {
            asset_vec[--i].destroy();
            asset_vec.pop_back();
        }
    }
    
    asset_load_return_t load_gltf_file(const std::string&);
    asset_load_return_t load_ktx2_file(const std::string&);

    std::vector<ve::mesh> meshes{};
    std::vector<ve::texture> textures{};
};

asset_manager& manager() noexcept;
}