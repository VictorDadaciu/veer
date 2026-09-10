#include "asset.h"

#include <veer_core/log.h>

#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tinygltf/tiny_gltf_v3.h>

#include <map>
#include <vector>

namespace ve
{
struct gltf_model_wrapper : public tg3_model
{
    VEER_DECLARE_NO_COPY(gltf_model_wrapper);

    error_code load(const std::filesystem::path& path) noexcept
    {
        std::string path_str = path.display_string();
        trace("Processing as gltf file...");
        
        tg3_error_stack errs;
        tg3_error_stack_init(&errs);
        if (tg3_parse_file(this, &errs, path_str.c_str(), path_str.size(), nullptr) != TG3_OK)
        {
            for (uint32_t i = 0; i < errs.count; ++i)
                warn(errs.entries[i].message ? errs.entries[i].message : "(null)");
            tg3_error_stack_free(&errs);
            return error(error_code::file_read, "Failed to load asset file \"{}\"", path_str);
        }

        tg3_error_stack_free(&errs);
        return error_code::success;
    }

    size_t get_buffer_and_update_buffers_if_needed(mesh& new_mesh, size_t buffer_index)
    {
        auto it = buffer_indices_map.find(buffer_index);
        if (it != buffer_indices_map.end())
            return it->second;
        
        size_t exact_byte_length = this->buffers[buffer_index].byte_length;
        new_mesh.m_buffer_views.push_back({
            .offset = total_size,
            .size = exact_byte_length
        });
        total_size += next_multiple_of_cache_line_size(exact_byte_length);
        return buffer_indices_map.insert({buffer_index, buffer_indices_map.size()}).first->second;
    }

    error_code allocate_and_copy(mesh& new_mesh)
    {
        try
        {
            SAFE_JUST_INIT(new_mesh.m_buffer, total_size);
            for (const auto& index_pairs : buffer_indices_map)
                memcpy(
                    reinterpret_cast<void*>(
                        const_cast<std::byte*>(
                            new_mesh.m_buffer.cpu.data + new_mesh.m_buffer_views[index_pairs.second].offset
                        )
                    ),
                    this->buffers[index_pairs.first].data.data,
                    new_mesh.m_buffer_views[index_pairs.second].size
                );
        }
        catch(...)
        {
            return error_code::allocation;
        }
        return error_code::success;
    }

    error_code initialize_mesh(mesh& new_mesh, size_t mesh_index)
    {
        buffer_indices_map.clear();
        total_size = 0zu;

        const tg3_mesh& mesh_spec = this->meshes[mesh_index];
        new_mesh.m_name = std::string(mesh_spec.name.data, mesh_spec.name.len);
        new_mesh.m_primitives.resize(mesh_spec.primitives_count);
        for (size_t i = 0; i < mesh_spec.primitives_count; ++i)
        {
            const tg3_primitive& primitive_spec = mesh_spec.primitives[i];
            mesh_primitive& new_primitive = new_mesh.m_primitives[i];
            new_primitive.m_parent_index = mesh_index;
            new_primitive.m_render_mode = static_cast<render_mode>(primitive_spec.mode);
            for (size_t j = 0; j < primitive_spec.attributes_count; ++j)
            {
                const tg3_str_int_pair& kv = primitive_spec.attributes[j];
                const tg3_accessor accessor = this->accessors[kv.value];
                const tg3_buffer_view& buffer_view = this->buffer_views[accessor.buffer_view];

                size_t buffer_index = get_buffer_and_update_buffers_if_needed(new_mesh, buffer_view.buffer);

                std::string attribute_name = std::string(kv.key.data, kv.key.len);
                new_primitive.m_vertex_attribute_views[attribute_name] = attribute_view{
                    .buffer = buffer_index,
                    .byte_offset = buffer_view.byte_offset + accessor.byte_offset,
                    .element_count = accessor.count,
                    .stride = buffer_view.byte_stride,
                    .component_size = static_cast<uint8_t>(tg3_component_size(accessor.component_type)),
                    .component_count = static_cast<uint8_t>(tg3_num_components(accessor.type)),
                    .floating_point = accessor.type >= TG3_COMPONENT_TYPE_FLOAT,
                };
            }
            if (primitive_spec.indices == -1)
                continue;

            const tg3_accessor accessor = this->accessors[primitive_spec.indices];
            const tg3_buffer_view& buffer_view = this->buffer_views[accessor.buffer_view];

            size_t buffer_index = get_buffer_and_update_buffers_if_needed(new_mesh, buffer_view.buffer);

            new_primitive.m_index_attribute_view = attribute_view{
                .buffer = buffer_index,
                .byte_offset = buffer_view.byte_offset + accessor.byte_offset,
                .element_count = accessor.count,
                .component_size = static_cast<uint8_t>(tg3_component_size(accessor.component_type)),
                .component_count = 1,
                .floating_point = false,
            };
        }
        return allocate_and_copy(new_mesh);
    }

    ~gltf_model_wrapper() { tg3_model_free(this); }

    std::map<size_t, size_t> buffer_indices_map{};
    size_t total_size{};
};

struct asset_manager
{
    VEER_DECLARE_NO_COPY(asset_manager);

    ~asset_manager() = default;

    void unload_all()
    {
        for (auto& mesh : meshes)
            mesh.destroy();
        meshes.clear();
    }

    // TODO: make this nicer
    void unload_range(size_t from, size_t to)
    {
        for (size_t i = from; i < to; ++i)
            meshes[i].destroy();
    }
    
    asset_load_return_t load_gltf_file(const std::filesystem::path& path)
    {
        std::vector<asset_load_metadata> res{};
        {
            gltf_model_wrapper model{};
            SAFE_RETURN_EXPECTED(model.load(path));
            if (model.meshes_count == 0)
                return std::unexpected(error(error_code::file_read, "Asset file \"{}\" is invalid: has no meshes", path));

            for (size_t i = 0; i < model.meshes_count; ++i)
            {
                mesh& new_mesh = meshes.emplace_back();
                if (model.initialize_mesh(new_mesh, i) != error_code::success)
                {
                    unload_range(meshes.size() - i - 1, meshes.size());
                    return std::unexpected(error(error_code::file_read, "Failed to read assset file \"{}\"", path));
                }

                res.push_back({
                    .name = new_mesh.m_name,
                    .path = path,
                    .index = meshes.size() - 1,
                    .type = asset_type::mesh,
                });
            }
            info("Successfully loaded {} mesh(es)", model.meshes_count);
        }
        return res;
    }

    std::vector<mesh> meshes;
} asset_mgr;
}

namespace ve::assets
{
asset_load_return_t load(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "Asset file \"{}\" doesn't exist", path));
    std::string extension = path.extension().display_string();
    info("Loading asset file at \"{}\"...", path);
    if (extension == ".glb" || extension == ".gltf")
    {
        return asset_mgr.load_gltf_file(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid asset file type \"{}\"", path));
    }
}

ve::mesh& mesh(const size_t& index)
{
    return asset_mgr.meshes[index];
}

void unload_all()
{
    asset_mgr.unload_all();
}
}