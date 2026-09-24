#include "staging_buffer.h"

#include "vk_command.h"
#include "vk_context.h"

#include <veer_core/log.h>

namespace ve
{
error_code staging_buffer::resize_if_needed(size_t s)
{
    if (s <= size)
        return error_code::success;

    trace("Resizing staging buffer to {} bytes", s);
    vk_buffer::destroy();

    size = s;
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };
    VmaAllocationCreateInfo buffer_alloc_create_info{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo alloc_info{};
    if (FAILED(vmaCreateBuffer(vk_context::get().allocator, &buffer_create_info, &buffer_alloc_create_info, &vk, alloc.write(), &alloc_info)))
        return error(error_code::allocation, "Failed to reallocate staging buffer");
    mapped = reinterpret_cast<std::byte*>(alloc_info.pMappedData);
    return error_code::success;
}

error_code staging_buffer::init(size_t size)
{
    SAFE_CALL(resize_if_needed(size));
    SAFE_JUST_INIT(fence);
    return error_code::success;
}

void staging_buffer::copy_to_mapped(const void* data, size_t size, size_t offset)
{
    memcpy(mapped + offset, data, size);
}

bool staging_buffer::is_transfering() const noexcept
{
    return !fence.is_signaled();
}

void staging_buffer::wait_until_finished_transfering() noexcept
{
    fence.wait();
}

error_code staging_buffer::transfer_to_buffer(const vk_buffer& target)
{
    wait_until_finished_transfering();

    vk_command_buffer SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE(cmd_buf, vk_context::get().allocate_cmd_buffer());
    SAFE_CALL(cmd_buf.begin());
    cmd_buf.copy_buffer(*this, target);
    SAFE_CALL(cmd_buf.end());
    fence.reset();
    SAFE_CALL(cmd_buf.submit(vk_context::get().queue, fence));
    // TODO: set some placeholder/index so the buffer will actually be used correctly when async
    return error_code::success;
}

error_code staging_buffer::transfer_to_image(const vk_image& target, const std::vector<VkBufferImageCopy2>& copy_regions)
{
    wait_until_finished_transfering();

    vk_command_buffer SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE(cmd_buf, vk_context::get().allocate_cmd_buffer());
    SAFE_CALL(cmd_buf.begin());
    
    VkImageMemoryBarrier2 barrier_tex_image{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = target,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = target.metadata.mip_levels, .layerCount = 1 }
    };

    VkDependencyInfo barrier_tex_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier_tex_image
    };
    cmd_buf.pipeline_barrier(barrier_tex_info);

    VkCopyBufferToImageInfo2 copy_buf_to_image_info{
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
        .srcBuffer = vk,
        .dstImage = target,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = static_cast<uint32_t>(copy_regions.size()),
        .pRegions = copy_regions.data()
    };
    cmd_buf.copy_buffer_to_image(copy_buf_to_image_info);

    VkImageMemoryBarrier2 barrier_tex_read{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        .image = target,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = target.metadata.mip_levels, .layerCount = 1 }
    };
    barrier_tex_info.pImageMemoryBarriers = &barrier_tex_read;
    cmd_buf.pipeline_barrier(barrier_tex_info);
    SAFE_CALL(cmd_buf.end());
    fence.reset();
    SAFE_CALL(cmd_buf.submit(vk_context::get().queue, fence));
    // TODO: set some placeholder/index so the buffer will actually be used correctly when async
    return error_code::success;
}

void staging_buffer::destroy()
{
    wait_until_finished_transfering();
    fence.destroy();
    vk_buffer::destroy();
}
}