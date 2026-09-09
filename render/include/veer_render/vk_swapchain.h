#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <vector>

struct VmaAllocation_T;
namespace ve
{
struct vk_swapchain_link
{
    DECLARE_NO_COPY(vk_swapchain_link);

    vk_swapchain_link(vk_swapchain_link&& other) :
        image(other.image),
        image_view(other.image_view),
        render_complete_semaphore(other.render_complete_semaphore)
    {
        other.image = nullptr;
        other.image_view = nullptr;
        other.render_complete_semaphore = nullptr;
    }

    vk_swapchain_link& operator=(vk_swapchain_link&& other)
    {
        image = other.image;
        image_view = other.image_view;
        render_complete_semaphore = other.render_complete_semaphore;
        other.image = nullptr;
        other.image_view = nullptr;
        other.render_complete_semaphore = nullptr;
        return *this;
    }

    error_code init(VkImage);
    void destroy();

    ~vk_swapchain_link() = default;
    
    VkImage image{};
    VkImageView image_view{};
    VkSemaphore render_complete_semaphore{};
};

class window;
struct vk_swapchain
{
    DECLARE_NO_COPY(vk_swapchain);

    vk_swapchain(vk_swapchain&& other) :
        vk(other.vk),
        extent(other.extent),
        links(std::move(other.links)),
        depth_format(other.depth_format),
        depth_image(other.depth_image),
        depth_image_view(other.depth_image_view),
        depth_image_alloc(other.depth_image_alloc)
    {
        other.vk = nullptr;
        other.links.clear();
        other.depth_image = nullptr;
        other.depth_image_view = nullptr;
        other.depth_image_alloc = nullptr;
    }
    vk_swapchain& operator=(vk_swapchain&& other)
    {
        vk = other.vk;
        extent = other.extent,
        links = std::move(other.links),
        depth_format = other.depth_format,
        depth_image = other.depth_image,
        depth_image_view = other.depth_image_view,
        depth_image_alloc = other.depth_image_alloc,
        other.vk = nullptr;
        other.links.clear();
        other.depth_image = nullptr;
        other.depth_image_view = nullptr;
        other.depth_image_alloc = nullptr;
        return *this;
    }

    error_code init(const window* win);
    void destroy();

    ~vk_swapchain() = default;

    VkSwapchainKHR vk{};
    VkExtent2D extent{};
    std::vector<vk_swapchain_link> links{};
    VkFormat depth_format;
    VkImage depth_image{};
    VkImageView depth_image_view{};
    VmaAllocation_T* depth_image_alloc{};
};
}
