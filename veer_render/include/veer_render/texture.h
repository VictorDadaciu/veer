#pragma once

#include "vk_image.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <cstdint>

struct VmaAllocation_T;
namespace ve
{
struct texture : public vk_sampled_image
{
    error_code init();
    using vk_sampled_image::destroy;

    vk_weak_ptr<VkDescriptorSet> descriptor{};
};
}