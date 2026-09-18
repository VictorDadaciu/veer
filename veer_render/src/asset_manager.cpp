#include "internal/asset_manager.h"

#include <veer_core/file.h>
#include <veer_core/log.h>

#include "internal/gltf.h"
#include "internal/ktx2.h"

namespace ve::assets
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

        for (size_t i = 0; i < model.meshes_count; ++i)
        {
            ve::mesh& new_mesh = meshes.emplace_back();
            if (model.initialize_mesh(new_mesh, i) != error_code::success)
            {
                unload_back(meshes, i);
                return std::unexpected(error(error_code::file_read, "Failed to read mesh file \"{}\"", path));
            }

            res.push_back({
                .name = new_mesh.name(),
                .index = meshes.size() - 1,
                .type = asset_type::mesh,
            });
        }
        info("Successfully loaded {} mesh(es) from \"{}\"", model.meshes_count, path);
    }
    return res;
}

asset_manager& manager() noexcept
{
    static asset_manager mgr{};
    return mgr;
}
}