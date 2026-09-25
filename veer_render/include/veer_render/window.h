#pragma once

#include "vk_swapchain.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

struct SDL_Window;
namespace ve
{
struct window;
struct vk_surface : public vk_unique_ptr<VkSurfaceKHR>
{
    error_code init(const window*);

    VkSurfaceCapabilitiesKHR capabilities{};
};

struct window : vk_unique_ptr<SDL_Window*>
{
public:
    [[nodiscard]] error_code open(c_string name, size_t width=1280zu, size_t height=720zu);
    void close();

    c_string title() const;

    size_t width() const;
    size_t height() const;
    error_code size(size_t& width, size_t& height) const;
    
    vk_surface surface{};
    vk_swapchain swapchain{};
};
}
