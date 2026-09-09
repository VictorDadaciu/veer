#include "mesh.h"

#include "asset.h"

#include <unordered_set>

namespace ve
{
static const std::unordered_set<std::string_view> attribute_names = {
    POSITION_ATTRIBUTE_NAME,
    NORMAL_ATTRIBUTE_NAME,
    TANGENT_ATTRIBUTE_NAME, 
    TEXCOORD_0_ATTRIBUTE_NAME,
    TEXCOORD_1_ATTRIBUTE_NAME,
    COLOR_0_ATTRIBUTE_NAME,
    JOINTS_0_ATTRIBUTE_NAME,  
    WEIGHTS_0_ATTRIBUTE_NAME, 
};

error_code mesh::upload_to_gpu()
{
    return m_buffer.upload_to_gpu();
}

void mesh::unload_from_gpu()
{
    m_buffer.unload_from_gpu();
}

void mesh::destroy()
{
    m_buffer.destroy();
}

mesh& mesh_primitive::parent() const noexcept
{
    return assets::mesh(m_parent_index);
}
}