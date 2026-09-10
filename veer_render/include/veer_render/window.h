#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include "vk_surface.h"
#include "vk_swapchain.h"

class SDL_Window;

namespace ve
{
class window
{
public:
    [[nodiscard]]
    error_code open(c_string name, size_t width=1280zu, size_t height=720zu);
    void close();

    ~window() = default;

    c_string title() const;

    size_t width() const;
    size_t height() const;
    error_code size(size_t& width, size_t& height) const;

private:
    friend class vk_context;
    friend class vk_surface;
    friend class vk_swapchain;
    
    SDL_Window* sdl{};
    vk_surface surface{};
    vk_swapchain swapchain{};
};
}
