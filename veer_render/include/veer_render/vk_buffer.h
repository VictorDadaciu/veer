#pragma once

#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <cstddef>

class VmaAllocation_T;
namespace ve
{
struct vk_buffer : public vk_allocated_ptr<VkBuffer>
{
    using vk_allocated_ptr<VkBuffer>::destroy;
    size_t size{};
};

struct persistently_mapped_buffer : vk_buffer
{
    using vk_buffer::destroy;
    std::byte* mapped{};
};
}