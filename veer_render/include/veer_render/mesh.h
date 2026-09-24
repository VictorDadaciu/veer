#pragma once

#include "vk_buffer.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ve
{
using namespace std::string_view_literals;
constexpr std::string_view POSITION_ATTRIBUTE_NAME   = "POSITION"sv;
constexpr std::string_view NORMAL_ATTRIBUTE_NAME     = "NORMAL"sv;
constexpr std::string_view TANGENT_ATTRIBUTE_NAME    = "TANGENT"sv;
constexpr std::string_view TEXCOORD_0_ATTRIBUTE_NAME = "TEXCOORD_0"sv;
constexpr std::string_view TEXCOORD_1_ATTRIBUTE_NAME = "TEXCOORD_1"sv;
constexpr std::string_view COLOR_0_ATTRIBUTE_NAME    = "COLOR_0"sv;
constexpr std::string_view JOINTS_0_ATTRIBUTE_NAME   = "JOINTS_0"sv;
constexpr std::string_view WEIGHTS_0_ATTRIBUTE_NAME  = "WEIGHTS_0"sv;

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

struct mesh;
struct mesh_primitive
{
    VEER_DECLARE_NO_COPY(mesh_primitive);

    mesh_primitive(mesh_primitive&&) = default;
    mesh_primitive& operator=(mesh_primitive&&) = default;

    mesh& parent() const noexcept;
    bool indexed() const noexcept { return index_attribute_view.element_count > 0; }

    std::unordered_map<std::string, attribute_view> vertex_attribute_views{};
    attribute_view index_attribute_view{};
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