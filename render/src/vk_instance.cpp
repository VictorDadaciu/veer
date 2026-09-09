#include "vk_instance.h"

#include "veer_core/log.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace ve
{
error_code vk_instance::init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return error(error_code::initialization, "Failed to initialize SDL3");

    uint32_t instance_extensions_count{0};
    char const* const* instance_extensions{SDL_Vulkan_GetInstanceExtensions(&instance_extensions_count)};

    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "veer_render",
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo instance_create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = instance_extensions_count,
        .ppEnabledExtensionNames = instance_extensions
    };

    if (FAILED(vkCreateInstance(&instance_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to initialize Vulkan instance");

    return error_code::success;
}

void vk_instance::destroy()
{
    SDL_Quit();
    vkDestroyInstance(vk, nullptr);
}
}
