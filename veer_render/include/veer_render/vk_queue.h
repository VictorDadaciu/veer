#pragma once

#include <veer_core/error_code.h>

#include <vulkan/vulkan_core.h>

namespace ve
{
struct vk_queue
{
    error_code init();

    VkQueue vk{};
    uint32_t family{};
};
}
