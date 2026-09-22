#include "vk_sync.h"

#include "vk_context.h"

#include <veer_core/log.h>

namespace ve
{
error_code vk_fence::init(bool signaled)
{
    VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u
    };
    if (FAILED(vkCreateFence(vk_context::get().device, &fence_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create fence");
    return error_code::success;
}

void vk_fence::destroy()
{
    wait();
    vk_unique_ptr<VkFence>::destroy();
}

void vk_fence::wait(size_t ns) const noexcept
{
    vkWaitForFences(vk_context::get().device, 1, &vk, true, ns);
}

bool vk_fence::is_signaled() const noexcept
{
    return vkGetFenceStatus(vk_context::get().device, vk) == VK_SUCCESS;
}

void vk_fence::reset() noexcept
{
    vkResetFences(vk_context::get().device, 1, &vk);
}

error_code vk_semaphore::init()
{
    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    if (FAILED(vkCreateSemaphore(vk_context::get().device, &semaphore_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create semaphore");
    return error_code::success;
}
}