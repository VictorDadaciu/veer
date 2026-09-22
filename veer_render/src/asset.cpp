#include "asset.h"

#include "vk_context.h"

#include "internal/asset_manager.h"

#include <veer_core/file.h>
#include <veer_core/log.h>

#include <assert.h>

namespace ve::assets
{
asset_load_return_t load(const std::string& path)
{
    if (!file::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "Asset file \"{}\" doesn't exist", path));
    std::string extension = file::extension(path);
    info("Loading asset file at \"{}\"...", path);

    auto& mgr = ve::asset_manager::get();
    if (extension == ".glb" || extension == ".gltf")
    {
        return mgr.load_gltf_file(path);
    }
    else if (extension == ".ktx2")
    {
        return mgr.load_ktx2_file(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid asset file type \"{}\"", path));
    }
}

ve::mesh& mesh(size_t index) noexcept
{
    assert(index < ve::asset_manager::get().meshes.size());
    return ve::asset_manager::get().meshes[index];
}

ve::texture& texture(size_t index) noexcept
{
    assert(index < ve::asset_manager::get().meshes.size());
    return ve::asset_manager::get().textures[index];
}

void unload_all() noexcept
{
    auto& mgr = ve::asset_manager::get();
    mgr.unload_back(mgr.meshes);
    mgr.unload_back(mgr.textures);
}
}