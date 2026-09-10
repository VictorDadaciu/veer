#pragma once

#include "buffer.h"

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
    size_t buffer{};
    size_t byte_offset{};
    size_t element_count{};
    size_t stride{};
    uint8_t component_size{};
    uint8_t component_count{};
    bool floating_point{};

    uint8_t element_size() const noexcept { return component_size * component_count; }
};

class mesh;
class mesh_primitive
{
public:
    mesh& parent() const noexcept;
    bool indexed() const noexcept { return m_index_attribute_view.element_count > 0; }

private:
    friend struct asset_manager;
    friend struct gltf_model_wrapper;
    friend class mesh;

    std::unordered_map<std::string, attribute_view> m_vertex_attribute_views{};
    attribute_view m_index_attribute_view{};
    size_t m_parent_index{};
    render_mode m_render_mode;
};

class mesh
{
public:
    std::string name() const noexcept { return m_name; }
    [[nodiscard]]
    error_code upload_to_gpu();
    void unload_from_gpu();
    
private:
    friend struct asset_manager;
    friend struct gltf_model_wrapper;
    friend class mesh_primitive;

    void destroy();

    std::vector<mesh_primitive> m_primitives{};
    std::vector<offset_span> m_buffer_views{};
    std::string m_name{};
    buffer m_buffer{};
};
}