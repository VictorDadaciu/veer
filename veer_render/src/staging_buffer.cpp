#include "internal/staging_buffer.h"

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

void staging_buffer::destroy()
{
    wait_until_finished_transfering();
    fence.destroy();
    vk_buffer::destroy();
}
}