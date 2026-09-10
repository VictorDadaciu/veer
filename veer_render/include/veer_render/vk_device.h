#pragma once

#include "vk_queue.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>


#include <vulkan/vulkan.h>

namespace ve
{
struct vk_context;

// TODO: show all physical devices available
struct vk_physical_device
{
    error_code init();

    VkPhysicalDevice vk{};
};

struct vk_device
{
    
    DECLARE_NO_COPY_NO_MOVE(vk_device);

    error_code init();
    void destroy();

    ~vk_device() = default;

    VkDevice vk{};
    vk_queue queue{};
};
}
