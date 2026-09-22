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
        .queueFamilyIndex = vk_context::get().queue.family
    };

    if (FAILED(vkCreateCommandPool(vk_context::get().device, &command_pool_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to initialize command pool");

    return error_code::success;
}

std::expected<vk_weak_ptr<VkCommandBuffer>, error_code> vk_command_pool::allocate_cmd_buffer(bool is_primary) const
{
    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk,
        .level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };

    vk_weak_ptr<VkCommandBuffer> cmd_buf{};
    if (FAILED(vkAllocateCommandBuffers(vk_context::get().device, &command_buffer_allocate_info, cmd_buf.write())))
        return std::unexpected(error(error_code::allocation, "Failed to allocate command buffer"));

    return cmd_buf;
}

std::expected<std::vector<vk_weak_ptr<VkCommandBuffer>>, error_code> vk_command_pool::allocate_cmd_buffers(uint32_t n, bool is_primary) const
{
    assert(n > 0);
    VkCommandBufferAllocateInfo command_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk,
        .level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = n,
    };

    std::vector<vk_weak_ptr<VkCommandBuffer>> cmd_bufs(n);
    if (FAILED(vkAllocateCommandBuffers(vk_context::get().device, &command_buffer_allocate_info, cmd_bufs.data()->write())))
        return std::unexpected(error(error_code::allocation, "Failed to allocate {} command buffers", n));
    return cmd_bufs;
}

error_code vk_command_buffer::begin(bool single_submit)
{
    VkCommandBufferBeginInfo cmd_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = single_submit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0u,
    };
    if (FAILED(vkBeginCommandBuffer(vk, &cmd_buffer_begin_info)))
        return error(error_code::command_record, "Failed to begin command buffer recording");
    return error_code::success;
}

error_code vk_command_buffer::end()
{
    if (FAILED(vkEndCommandBuffer(vk)))
        return error(error_code::command_record, "Failed to end command buffer recording");
    return error_code::success;
}

error_code vk_command_buffer::submit(vk_weak_ptr<VkQueue> queue, vk_weak_ptr<VkFence> fence)
{
    VkCommandBufferSubmitInfo cmd_buffer_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = vk
    };
    VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_buffer_submit_info,
    };
    if (FAILED(vkQueueSubmit2(queue, 1, &submit_info, fence)))
        return error(error_code::render_submit, "Failed to submit command buffer");
    return error_code::success;
}

void vk_command_buffer::copy_buffer(const vk_buffer& source, const vk_buffer& target, size_t size)
{
    VkBufferCopy2 region{
        .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .size = size,
    };
    VkCopyBufferInfo2 copy_buffer_info{
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .srcBuffer = source,
        .dstBuffer = target,
        .regionCount = 1u,
        .pRegions = &region
    };
    vkCmdCopyBuffer2(vk, &copy_buffer_info);
}
}