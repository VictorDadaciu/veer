#include "vk_command.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <assert.h>

namespace ve
{
error_code vk_command_pool::init(VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = context.device.queue.family
    };

    if (FAILED(vkCreateCommandPool(context.device.vk, &command_pool_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to initialize command pool");

    return error_code::success;
}

void vk_command_pool::destroy()
{
    vkDestroyCommandPool(context.device.vk, vk, nullptr);
}

std::expected<VkCommandBuffer, error_code> vk_command_pool::allocate_cmd_buffer(bool is_primary) const
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk,
        .level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd_buf{};
    if (FAILED(vkAllocateCommandBuffers(context.device.vk, &command_buffer_allocate_info, &cmd_buf)))
        return std::unexpected(error(error_code::allocation, "Failed to allocate command buffer"));

    return cmd_buf;
}

std::expected<std::vector<VkCommandBuffer>, error_code> vk_command_pool::allocate_cmd_buffers(bool is_primary, uint32_t n) const
{
    assert(n > 0);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk,
        .level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = n,
    };

    std::vector<VkCommandBuffer> cmd_bufs(n);
    if (FAILED(vkAllocateCommandBuffers(context.device.vk, &command_buffer_allocate_info, cmd_bufs.data())))
        return std::unexpected(error(error_code::allocation, "Failed to allocate {} command buffers", n));

    return cmd_bufs;
}
}