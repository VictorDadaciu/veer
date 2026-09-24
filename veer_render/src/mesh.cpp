#include "mesh.h"

#include "assets.h"

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

mesh& mesh_primitive::parent() const noexcept
{
    return assets::mesh(parent_index);
}
}