#pragma once

#include "vk_buffer.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ve
{
const std::string POSITION_ATTRIBUTE_NAME   = "POSITION";
const std::string NORMAL_ATTRIBUTE_NAME     = "NORMAL";
const std::string TANGENT_ATTRIBUTE_NAME    = "TANGENT";
const std::string TEXCOORD_0_ATTRIBUTE_NAME = "TEXCOORD_0";
const std::string TEXCOORD_1_ATTRIBUTE_NAME = "TEXCOORD_1";
const std::string COLOR_0_ATTRIBUTE_NAME    = "COLOR_0";
const std::string JOINTS_0_ATTRIBUTE_NAME   = "JOINTS_0";
const std::string WEIGHTS_0_ATTRIBUTE_NAME  = "WEIGHTS_0";

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

struct attribute_view
{
    size_t buffer_view_index{};
    size_t byte_offset{};
    size_t element_count{};
    size_t stride{};
    uint8_t component_size{};
    uint8_t component_count{};
    bool is_floating_point{};

    uint8_t element_size() const noexcept { return component_size * component_count; }
};

inline VkIndexType as_index_type(size_t size)
{
    switch (size)
    {
        case 2: return VK_INDEX_TYPE_UINT16;
        case 4: return VK_INDEX_TYPE_UINT32;
        default:
            assert(false);
            return VK_INDEX_TYPE_NONE_KHR;
    }
}

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
}