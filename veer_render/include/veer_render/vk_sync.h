#pragma once

#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <limits>

namespace ve
{
struct vk_fence : public vk_unique_ptr<VkFence>
{
    error_code init(bool=true);
    void destroy();

    // nanoseconds
    void wait(size_t=std::numeric_limits<size_t>::max()) const noexcept;
    bool is_signaled() const noexcept;
    void reset() noexcept;
};

struct vk_semaphore : public vk_unique_ptr<VkSemaphore>
{
    error_code init();
};
}