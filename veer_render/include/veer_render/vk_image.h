#pragma once

#include "vk_ptr.h"

#include <vulkan/vulkan.h>

namespace ve
{
struct image_metadata
{
    size_t width{};
    size_t height{};
    size_t size{};
    VkFormat format{};
    uint8_t mip_levels = 1;
};

struct vk_image : public vk_allocated_ptr<VkImage>
{
    using vk_allocated_ptr<VkImage>::destroy;

    size_t width() const noexcept { return metadata.width; }
    size_t height() const noexcept { return metadata.height; }
    size_t size() const noexcept { return metadata.size; }
    VkFormat format() const noexcept { return metadata.format; }
    uint8_t mip_levels() const noexcept { return metadata.mip_levels; }

    image_metadata metadata{};
};

struct vk_viewed_image : public vk_image
{
    void destroy()
    {
        view.destroy();
        vk_image::destroy();
    }

    vk_unique_ptr<VkImageView> view{};
};

struct vk_sampled_image : public vk_viewed_image
{
    void destroy()
    {
        sampler.destroy();
        vk_viewed_image::destroy();
    }

    vk_unique_ptr<VkSampler> sampler{};
};
}