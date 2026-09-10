#include "window.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <string>

namespace ve
{
error_code window::open(c_string name, size_t width, size_t height)
{
    trace("Creating window \"{}\"", name);
    sdl = SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!sdl)
        return error_code::initialization;
    SAFE_JUST_INIT(surface, this);
    SAFE_JUST_INIT(swapchain, this);

    return error_code::success;
}

c_string window::title() const
{
    return SDL_GetWindowTitle(sdl);
}

error_code window::size(size_t& width, size_t& height) const
{
    int w{}, h{};
    if (!SDL_GetWindowSize(sdl, &w, &h))
    {
        warn("SDL_GetWindowSize failed");
        return error_code::window;
    }
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
    trace("Closing window \"{}\"", title());
    vkDeviceWaitIdle(context.device.vk);
    swapchain.destroy();
    surface.destroy();
    SDL_DestroyWindow(sdl);
}
}
