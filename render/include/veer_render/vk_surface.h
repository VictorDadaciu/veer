#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

namespace ve
{
class window;
struct vk_surface
{
    DECLARE_NO_COPY(vk_surface);

    vk_surface(vk_surface&& other) :
        vk(other.vk)
    {
        other.vk = nullptr;
    }
    vk_surface& operator=(vk_surface&& other)
    {
        vk = other.vk;
        other.vk = nullptr;
        return *this;
    }

    error_code init(const window* win);
    void destroy();

    ~vk_surface() = default;

    VkSurfaceKHR vk{};
    VkSurfaceCapabilitiesKHR capabilities{};
};
}
