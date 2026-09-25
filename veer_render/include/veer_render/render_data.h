#pragma once

#include "vk_buffer.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <glm/glm.hpp>

namespace ve
{
struct shader_data
{
    glm::mat4 proj{};
    glm::mat4 view{};
    glm::mat4 model[3];
    glm::vec4 light_pos{ 0.0f, -10.0f, 10.0f, 0.0f };
    uint32_t selected{1};
};

struct render_data
{
    VEER_DECLARE_NO_COPY(render_data);
    render_data(render_data&&) = default;
    render_data& operator=(render_data&&) = default;

    error_code init();
    void destroy();

    shader_data data{};
    persistently_mapped_buffer ubo{};
    VkDeviceAddress device_address{};
};
}