#include "window.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <string>

namespace ve
{
error_code vk_surface::init(const window* win)
{
    if (!SDL_Vulkan_CreateSurface(*win, vk_context::get().instance, nullptr, &vk) ||
        LEGACY_FAILED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context::get().gpu(), vk, &capabilities)))
        return error(error_code::initialization, "Failed to create render surface");

    return error_code::success;
}

error_code window::open(c_string name, size_t width, size_t height)
{
    info("Opening window \"{}\"", name);
    vk = SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!vk)
        return error(error_code::initialization, "Failed to create SDL window");
    
    SAFE_JUST_INIT(surface, this);
    SAFE_JUST_INIT(swapchain, this);

    return error_code::success;
}

c_string window::title() const
{
    return SDL_GetWindowTitle(vk);
}

glm::uvec2 window::size() const
{
    int w{}, h{};
    glm::uvec2 ret{};
    if (!SDL_GetWindowSize(vk, &w, &h))
    {
        warn("SDL_GetWindowSize failed");
        return ret;
    }
    ret.x = static_cast<uint32_t>(w);
    ret.y = static_cast<uint32_t>(h);
    return ret;
}

void window::close()
{
    info("Closing window \"{}\"", title());
    vk_context::get().device.wait_idle();
    swapchain.destroy();
    surface.destroy();
    vk_unique_ptr<SDL_Window*>::destroy();
}
}
