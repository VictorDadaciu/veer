#include "vk_context.h"

#include <veer_core/log.h>

namespace ve
{
vk_context context;

error_code vk_frame_context::init()
{
    if (command_pool) return error_code::already_initialized;

    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = context.device.queue.family
    };

    if (FAILED(vkCreateCommandPool(context.device.vk, &command_pool_create_info, nullptr, &command_pool)))
        return error(error_code::initialization, "Failed to initialize command pool");

    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .commandBufferCount = 1
    };

    if (FAILED(vkAllocateCommandBuffers(context.device.vk, &command_buffer_allocate_info, &command_buffer)))
        return error(error_code::allocation, "Failed to allocate command buffer");

    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    if (FAILED(vkCreateSemaphore(context.device.vk, &semaphore_create_info, nullptr, &image_acquired_semaphore)))
        return error(error_code::initialization, "Failed to create semaphore");

    VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    if (FAILED(vkCreateFence(context.device.vk, &fence_create_info, nullptr, &render_start_fence)))
        return error(error_code::initialization, "Failed to create fence");

    return error_code::success;
}

void vk_frame_context::destroy()
{
    if (!command_pool) return;
    vkDestroySemaphore(context.device.vk, image_acquired_semaphore, nullptr);
    vkDestroyFence(context.device.vk, render_start_fence, nullptr);
    vkDestroyCommandPool(context.device.vk, command_pool, nullptr);
    command_pool = nullptr;
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
    if (!instance.vk) return;

    vkDeviceWaitIdle(context.device.vk);
    for (auto& frame : frames)
        frame.destroy();
    allocator.destroy();
    device.destroy();
    instance.destroy();
}
}
