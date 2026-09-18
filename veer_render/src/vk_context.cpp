#include "vk_context.h"

#include <veer_core/log.h>

namespace ve
{
// TODO: add validator callbacks and stuff
error_code vk_frame_context::init()
{
    SAFE_JUST_INIT(pool);
    SAFE_CALL_HANDLE_EXPECTED(command_buffer, pool.allocate_cmd_buffer());

    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    if (FAILED(vkCreateSemaphore(vk::context().device.vk, &semaphore_create_info, nullptr, &image_acquired_semaphore)))
        return error(error_code::initialization, "Failed to create semaphore");

    VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    if (FAILED(vkCreateFence(vk::context().device.vk, &fence_create_info, nullptr, &render_start_fence)))
        return error(error_code::initialization, "Failed to create fence");

    return error_code::success;
}

void vk_frame_context::destroy()
{
    vkDestroySemaphore(vk::context().device.vk, image_acquired_semaphore, nullptr);
    vkDestroyFence(vk::context().device.vk, render_start_fence, nullptr);
    pool.destroy();
}

error_code vk_context::init()
{
    SAFE_JUST_INIT(instance);
    SAFE_JUST_INIT(physical_device);
    SAFE_JUST_INIT(device);
    SAFE_JUST_INIT(allocator);
    for (auto& frame : frames)
        SAFE_JUST_INIT(frame);
    return error_code::success;
}

void vk_context::destroy()
{
    for (auto& frame : frames)
        frame.destroy();
    allocator.destroy();
    device.destroy();
    instance.destroy();
}

vk_context& vk::context() noexcept
{
    static vk_context ctx;
    return ctx;
}
}
