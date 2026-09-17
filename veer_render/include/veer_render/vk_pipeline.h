#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

namespace ve
{
struct vk_shader_module
{
    VEER_DECLARE_NO_COPY(vk_shader_module);

    vk_shader_module(vk_shader_module&& other)
    : vk(other.vk)
    {
        other.vk = nullptr;
    }

    vk_shader_module& operator=(vk_shader_module&& other)
    {
        vk = other.vk;
        other.vk = nullptr;
        return *this;
    }

    error_code init(const byte_span&);
    void destroy();

    ~vk_shader_module() = default;

    VkShaderModule vk{};
};

struct vk_pipeline
{
    VEER_DECLARE_NO_COPY(vk_pipeline);

    vk_pipeline(vk_pipeline&& other)
    : vk(other.vk)
    {
        other.vk = nullptr;
    }

    vk_pipeline& operator=(vk_pipeline&& other)
    {
        vk = other.vk;
        other.vk = nullptr;
        return *this;
    }

    error_code init(const byte_span&);
    error_code init(const vk_shader_module&);
    void destroy();

    ~vk_pipeline() = default;

    VkPipeline vk{};
};
}