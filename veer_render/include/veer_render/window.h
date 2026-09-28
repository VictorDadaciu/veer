#pragma once

#include "vk_swapchain.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <glm/glm.hpp>

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

    glm::uvec2 size() const;
    size_t width() const { return size().x; }

    size_t height() const { return size().y; }

    float aspect_ratio() const
    {
        auto s = size();
        return s.y > 0 ? static_cast<float>(s.x) / static_cast<float>(s.y) : 0.f;
    }
    
    vk_surface surface{};
    vk_swapchain swapchain{};
};
}
