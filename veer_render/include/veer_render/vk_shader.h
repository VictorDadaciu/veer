#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <filesystem>

namespace ve
{
struct vk_shader_module
{
    VEER_DECLARE_NO_COPY_NO_MOVE(vk_shader_module);

    error_code init(const std::filesystem::path&);
    void destroy();

    ~vk_shader_module() = default;

    byte_span code{};
    VkShaderModule vk{};
};
}