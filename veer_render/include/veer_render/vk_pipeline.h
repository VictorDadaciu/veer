#pragma once

#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

namespace ve
{
struct vk_shader_module : public vk_unique_ptr<VkShaderModule>
{
    VEER_DECLARE_NO_COPY(vk_shader_module);
    vk_shader_module(vk_shader_module&& other) = default;
    vk_shader_module& operator=(vk_shader_module&&) = default;

    error_code init(const byte_span&);
    using vk_unique_ptr<VkShaderModule>::destroy;
};

struct vk_pipeline : vk_unique_ptr<VkPipeline>
{
    VEER_DECLARE_NO_COPY(vk_pipeline);
    vk_pipeline(vk_pipeline&&) = default;
    vk_pipeline& operator=(vk_pipeline&&) = default;

    error_code init(const byte_span&);
    error_code init(const vk_shader_module&);
    
    void destroy()
    {
        vk_unique_ptr<VkPipeline>::destroy();
        layout.destroy();
    }

    vk_unique_ptr<VkPipelineLayout> layout{};
};
}