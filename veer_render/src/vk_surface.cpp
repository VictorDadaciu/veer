#include "vk_surface.h"

#include "vk_context.h"
#include "window.h"

#include <veer_core/log.h>

#include <SDL3/SDL_vulkan.h>

namespace ve
{
error_code vk_surface::init(const window* win)
{
    if (!SDL_Vulkan_CreateSurface(win->sdl, context.instance.vk, nullptr, &vk) ||
        FAILED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physical_device.vk, vk, &capabilities)))
        return error(error_code::initialization, "Failed to create render surface");

    return error_code::success;
}

void vk_surface::destroy()
{
    SDL_Vulkan_DestroySurface(context.instance.vk, vk, nullptr);
}
}
