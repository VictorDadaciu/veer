#include "assets.h"

#include "vk_context.h"

#include "internal/asset_manager.h"
#include "internal/gltf.h"
#include "internal/ktx2.h"

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

namespace ve
{
asset_load_return_t asset_manager::load_ktx2_file(const std::string& path)
{
    std::vector<asset_load_metadata> res{};
    {
        ktx2_texture_wrapper tex{};
        SAFE_CALL_RETURN_EXPECTED(tex.load(path));

        ve::texture& new_tex = textures.emplace_back();
        if (tex.initialize_texture(new_tex) != error_code::success)
        {
            unload_back(textures, 1);
            return std::unexpected(error(error_code::file_read, "Failed to read texture file \"{}\"", path));
        }

        res.push_back({
            .name = file::stem(path),
            .index = textures.size() - 1,
            .type = asset_type::texture
        });
        info("Successfully loaded texture from \"{}\"", path);
    }
    return res;
}

asset_load_return_t asset_manager::load_gltf_file(const std::string& path)
{
    std::vector<asset_load_metadata> res{};
    {
        gltf_model_wrapper model{};
        SAFE_CALL_RETURN_EXPECTED(model.load(path));
        if (model.meshes_count == 0)
            return std::unexpected(error(error_code::file_read, "Asset file \"{}\" is invalid: has no meshes", path));

        trace("Loaded .gltf file");
        for (size_t i = 0; i < model.meshes_count; ++i)
        {
            ve::mesh& new_mesh = meshes.emplace_back();
            if (model.initialize_mesh(new_mesh, i) != error_code::success)
            {
                unload_back(meshes, i);
                return std::unexpected(error(error_code::file_read, "Failed to read mesh file \"{}\"", path));
            }

            res.push_back({
                .name = new_mesh.name,
                .index = meshes.size() - 1,
                .type = asset_type::mesh,
            });
        }
        info("Successfully loaded {} mesh(es) from \"{}\"", model.meshes_count, path);
    }
    return res;
}
}