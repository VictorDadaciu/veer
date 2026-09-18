#define TINYGLTF3_IMPLEMENTATION
#include "internal/gltf.h"

#include "internal/asset_manager.h"

#include <veer_core/file.h>
#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

namespace ve::assets
{
error_code gltf_model_wrapper::load(const std::string& path)
{
    trace("Processing as gltf file...");
    
    tg3_error_stack errs;
    tg3_error_stack_init(&errs);
    if (tg3_parse_file(this, &errs, path.c_str(), path.size(), nullptr) != TG3_OK)
    {
        for (uint32_t i = 0; i < errs.count; ++i)
            warn(errs.entries[i].message ? errs.entries[i].message : "(null)");
        tg3_error_stack_free(&errs);
        return error(error_code::file_read, "Failed to load asset file \"{}\"", path);
    }

    tg3_error_stack_free(&errs);
    return error_code::success;
}

size_t gltf_model_wrapper::get_buffer_and_update_buffers_if_needed(ve::mesh& new_mesh, size_t buffer_index)
{
    auto it = m_buffer_indices_map.find(buffer_index);
    if (it != m_buffer_indices_map.end())
        return it->second;
    
    size_t exact_byte_length = this->buffers[buffer_index].byte_length;
    new_mesh.m_buffer_views.push_back({
        .offset = m_total_size,
        .size = exact_byte_length
    });
    m_total_size += next_multiple_of_cache_line_size(exact_byte_length);
    return m_buffer_indices_map.insert({buffer_index, m_buffer_indices_map.size()}).first->second;
}

void gltf_model_wrapper::allocate_and_copy(ve::mesh& new_mesh)
{
    new_mesh.m_buffer.init(m_total_size, buffer_type::vertex_and_index);
    for (const auto& index_pairs : m_buffer_indices_map)
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

error_code gltf_model_wrapper::initialize_mesh(ve::mesh& new_mesh, size_t mesh_index)
{
    m_buffer_indices_map.clear();
    m_total_size = 0zu;

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
    allocate_and_copy(new_mesh);
    return error_code::success;
}
}