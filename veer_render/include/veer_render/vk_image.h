#pragma once

#include "vk_ptr.h"

#include <vulkan/vulkan.h>

namespace ve
{
struct vk_viewed_image : public vk_allocated_ptr<VkImage>
{
    void destroy()
    {
        view.destroy();
        vk_allocated_ptr<VkImage>::destroy();
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