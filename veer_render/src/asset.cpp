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
    if (extension == ".glb" || extension == ".gltf")
    {
        return manager().load_gltf_file(path);
    }
    else if (extension == ".ktx2")
    {
        return manager().load_ktx2_file(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid asset file type \"{}\"", path));
    }
}

ve::mesh& mesh(size_t index) noexcept
{
    assert(index < manager().meshes.size());
    return manager().meshes[index];
}

ve::texture& texture(size_t index) noexcept
{
    assert(index < manager().meshes.size());
    return manager().textures[index];
}

void unload_all() noexcept
{
    manager().unload_back(manager().meshes);
    manager().unload_back(manager().textures);
}
}