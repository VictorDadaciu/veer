#pragma once

#include "vk_image.h"
#include "vk_sync.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>

namespace ve
{
struct vk_swapchain_link
{
    error_code init(vk_weak_ptr<VkImage>);
    void destroy();
    
    vk_weak_ptr<VkImage> image{};
    vk_unique_ptr<VkImageView> image_view{};
    vk_semaphore render_complete_semaphore{};
};

class window;
struct vk_swapchain : public vk_unique_ptr<VkSwapchainKHR>
{
    error_code init(const window* win);
    void destroy();

    vk_swapchain_link& acquire_next(const vk_semaphore&);
    vk_swapchain_link& current_link() noexcept
    {
        assert(current_image_index < links.size());
        return links[current_image_index];
    }
    const vk_swapchain_link& current_link() const noexcept
    {
        assert(current_image_index < links.size());
        return links[current_image_index];
    }

    std::vector<vk_swapchain_link> links{};
    VkExtent2D extent{};
    vk_viewed_image depth_image{};
    VkFormat image_format;
    VkFormat depth_format;
    uint32_t current_image_index{};
};
}
