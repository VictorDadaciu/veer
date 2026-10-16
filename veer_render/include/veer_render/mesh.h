#pragma once

#include <veer_core/db.h>

#include "vk_buffer.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ve
{
enum attribute_type : uint8_t
{
    e_index,
    e_position,
    e_normal,
    e_texcoord_0,
    e_texcoord_1,
    e_color_0,
    e_tangent,
    e_joints_0,
    e_weights_0,
};

const std::string POSITION_ATTRIBUTE_NAME   = "POSITION";
const std::string NORMAL_ATTRIBUTE_NAME     = "NORMAL";
const std::string TANGENT_ATTRIBUTE_NAME    = "TANGENT";
const std::string TEXCOORD_0_ATTRIBUTE_NAME = "TEXCOORD_0";
const std::string TEXCOORD_1_ATTRIBUTE_NAME = "TEXCOORD_1";
const std::string COLOR_0_ATTRIBUTE_NAME    = "COLOR_0";
const std::string JOINTS_0_ATTRIBUTE_NAME   = "JOINTS_0";
const std::string WEIGHTS_0_ATTRIBUTE_NAME  = "WEIGHTS_0";
const std::string INDEX_ATTRIBUTE_NAME      = "INDEX";

enum class render_mode : uint8_t
{
    points,
    lines,
    line_loop,
    line_strip,
    triangles,
    triangle_strip,
    triangle_fan,
};

struct mesh_component_metadata
{
    uint8_t size_exp : 2; // this is the power to which to raise 2 to get the actual size, use size()
    uint8_t count : 5;
    uint8_t integral : 1;

    uint8_t component_size() const noexcept { return 1 << size_exp; }
    uint8_t element_size() const noexcept { return count << size_exp; }

    VkIndexType index_type() const noexcept
    {
        switch (size_exp)
        {
            case 1: return VK_INDEX_TYPE_UINT16;
            case 2: return VK_INDEX_TYPE_UINT32;
            default:
                assert(false);
                return VK_INDEX_TYPE_NONE_KHR;
        }
    }
};

struct attribute_view
{
    size_t buffer_view_index{};
    size_t byte_offset{};
    size_t element_count{};
    uint8_t stride{};
    mesh_component_metadata comp{};
};

struct mesh;
struct mesh_primitive
{
    VEER_DECLARE_NO_COPY(mesh_primitive);

    mesh_primitive(mesh_primitive&&) = default;
    mesh_primitive& operator=(mesh_primitive&&) = default;

    mesh& parent() const noexcept;
    bool is_indexed() const noexcept { return index_attribute_view.element_count > 0; }

    std::unordered_map<std::string, attribute_view> vertex_attribute_views{};
    attribute_view index_attribute_view{};
    size_t vertex_count{};
    size_t parent_index{};
    render_mode mode;
};

class mesh : public vk_buffer
{
public:
    VEER_DECLARE_NO_COPY(mesh);

    mesh(mesh&&) = default;
    mesh& operator=(mesh&&) = default;

    using vk_buffer::destroy;

    std::vector<mesh_primitive> primitives{};
    std::vector<offset_span> buffer_views{};
    std::string name{};
};

VEER_DERIVE_SORTED_PROP(attribute_view_handle_p,   VEER_PROPERTY_ROOT(handle));
VEER_DERIVE_SORTED_PROP(mesh_primitive_handle_p,   VEER_PROPERTY_ROOT(handle));
VEER_DERIVE_SORTED_PROP(mesh_handle_p,             VEER_PROPERTY_ROOT(handle));

VEER_DERIVE_PROP(byte_offset_p,         VEER_PROPERTY_ROOT(size_t));
VEER_DERIVE_PROP(element_count_p,       VEER_PROPERTY_ROOT(size_t));

VEER_DERIVE_PROP(buffer_view_index_p,   VEER_PROPERTY_ROOT(uint8_t));
VEER_DERIVE_PROP(byte_stride_p,         VEER_PROPERTY_ROOT(uint8_t));

VEER_DERIVE_PROP(component_metadata_p,  VEER_PROPERTY_ROOT(mesh_component_metadata));

VEER_DERIVE_PROP(attribute_type_p,      VEER_PROPERTY_ROOT(attribute_type));

using attribute_views_table = VEER_TABLE(
    VEER_COLUMN_IMPL(mesh_primitive_handle_p),
    VEER_PACKED_COLUMN(
        VEER_IMPL(buffer_view_index_p),
        VEER_IMPL(attribute_type_p)
    ),
    VEER_PACKED_COLUMN(
        VEER_IMPL(byte_offset_p),
        VEER_IMPL(element_count_p)
    ),
    VEER_PACKED_COLUMN(
        VEER_IMPL(byte_stride_p),
        VEER_IMPL(component_metadata_p)
    )
);

VEER_DERIVE_PROP(mesh_index_p,              VEER_PROPERTY_ROOT(uint16_t));
VEER_DERIVE_PROP(vertex_count_p,            VEER_PROPERTY_ROOT(size_t));

VEER_DERIVE_PROP(render_mode_p,   VEER_PROPERTY_ROOT(render_mode));

VEER_DERIVE_PROP(vk_buffer_p,       VEER_PROPERTY_ROOT(vk_buffer));

VEER_DERIVE_PROP(buffer_views_p,    VEER_PROPERTY_ROOT(std::vector<offset_span>));

VEER_DERIVE_PROP(mesh_name_p,       VEER_PROPERTY_ROOT(std::string));

using mesh_primitives_table = VEER_TABLE(
    VEER_COLUMN_IMPL(mesh_index_p),
    VEER_COLUMN_IMPL(vertex_count_p),
    VEER_COLUMN_IMPL(attribute_view_handle_p),
    VEER_COLUMN_IMPL(mesh_handle_p),
    VEER_COLUMN_IMPL(render_mode_p)
);

using mesh_table = VEER_TABLE(
    VEER_COLUMN_IMPL(vk_buffer_p),
    VEER_COLUMN_IMPL(buffer_views_p),
    VEER_COLUMN_IMPL(mesh_primitive_handle_p),
    VEER_COLUMN_IMPL(mesh_name_p)
);

using mesh_db = database<
    attribute_views_table,
    mesh_primitives_table,
    mesh_table
>;
}