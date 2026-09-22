#include "buffer.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

namespace ve
{
error_code mesh_buffer::init(size_t s)
{
    size = s;
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };
    VmaAllocationCreateInfo buffer_alloc_create_info{
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (FAILED(vmaCreateBuffer(vk_context::get().allocator, &buffer_create_info, &buffer_alloc_create_info, &vk, alloc.write(), nullptr)))
        return error(error_code::allocation, "Failed to allocate mesh buffer");
    return error_code::success;
}
}