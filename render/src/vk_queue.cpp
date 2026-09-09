#include "vk_queue.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <SDL3/SDL_vulkan.h>

#include <vector>

namespace ve
{
error_code vk_queue::init()
{
    uint32_t queue_family_count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(context.physical_device.vk, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physical_device.vk, &queue_family_count, queue_families.data());

    bool found = false;
    for (size_t i = 0; i < queue_families.size(); ++i)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            !FAILED(SDL_Vulkan_GetPresentationSupport(context.instance.vk, context.physical_device.vk, i)))
        {
            family = i;
            found = true;
            break;
        }
    }

    if (!found)
        return error(error_code::initialization, "Failed to find appropriate queue on physical device");

    return error_code::success;
}
}
