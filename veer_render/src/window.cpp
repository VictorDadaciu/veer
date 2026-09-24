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
        FAILED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context::get().gpu(), vk, &capabilities)))
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

error_code window::size(size_t& width, size_t& height) const
{
    int w{}, h{};
    if (!SDL_GetWindowSize(vk, &w, &h))
        return warn(error_code::window, "SDL_GetWindowSize failed");
    width = static_cast<size_t>(w);
    height = static_cast<size_t>(h);
    return error_code::success;
}

size_t window::width() const
{
    size_t w{}, h{};
    if (size(w, h) == error_code::window)
        return 0;
    return w;
}

size_t window::height() const
{
    size_t w{}, h{};
    if (size(w, h) == error_code::window)
        return 0;
    return h;
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
