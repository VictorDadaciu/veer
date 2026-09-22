#pragma once

#include "vk_image.h"
#include "vk_sync.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <vector>

struct VmaAllocation_T;
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

    std::vector<vk_swapchain_link> links{};
    VkExtent2D extent{};
    VkFormat depth_format;
    vk_viewed_image depth_image{};
};
}
