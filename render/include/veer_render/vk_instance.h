#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan_core.h>

namespace ve
{
struct vk_instance
{
    DECLARE_SINGLE_COPY(vk_instance);

    error_code init();
    void destroy();

    ~vk_instance() = default;

    VkInstance vk{};
};
}
