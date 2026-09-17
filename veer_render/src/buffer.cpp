#include "buffer.h"

#include "vk_context.h"

#include <veer_core/log.h>
#include <veer_core/utils.h>

#include <vma/vk_mem_alloc.h>

#include <assert.h>
#include <cstring>

namespace
{
using namespace ve;
VkBufferCreateFlags buffer_type_to_vk_buffer_usage_flags(buffer_type type)
{
    switch (type)
    {
    case buffer_type::vertex:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case buffer_type::index:
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case buffer_type::vertex_and_index:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case buffer_type::image_transfer_src:
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        break;
    default:
        warn("Unknown buffer type index \"{}\"", static_cast<size_t>(type));
        assert(false);
        break;
    }
}
VmaAllocationCreateFlags buffer_type_to_vma_alloc_create_flags(buffer_type type)
{
    switch (type)
    {
    case buffer_type::vertex:
    case buffer_type::index:
    case buffer_type::vertex_and_index:
        return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    case buffer_type::image_transfer_src:
        return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
            VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    default:
        warn("Unknown buffer type index \"{}\"", static_cast<size_t>(type));
        assert(false);
        break;
    }
}
}

namespace ve
{
error_code vk_buffer::upload(const std::byte* data, VkDeviceSize size, VkBufferUsageFlags usage_flags, VmaAllocationCreateFlags alloc_flags)
{
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags
    };

    // TODO: staging buffer
    VmaAllocationCreateInfo buffer_alloc_create_info{
        .flags = alloc_flags,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo buffer_alloc_info{};
    if (FAILED(vmaCreateBuffer(context.allocator.vk, &buffer_create_info, &buffer_alloc_create_info, &vk, &alloc, &buffer_alloc_info)))
        return error(error_code::allocation, "Failed to allocate buffer on device");

    memcpy(buffer_alloc_info.pMappedData, reinterpret_cast<const void*>(data), size);
    return error_code::success;
}

void vk_buffer::destroy()
{
    vkDeviceWaitIdle(context.device.vk);
    vmaDestroyBuffer(context.allocator.vk, vk, alloc);
}

error_code buffer::init(const std::byte* data, size_t size, buffer_type type)
{
    destroy();
    cpu.data = data;
    cpu.size = size;
    this->type = type;
    return error_code::success;
}

error_code buffer::init(const byte_span& data, buffer_type type)
{
    destroy();
    cpu = data;
    this->type = type;
    return error_code::success;
}

error_code buffer::init(size_t size, buffer_type type)
{
    destroy();
    cpu = byte_span{
        .data = allocate_cache_aligned_bytes(size),
        .size = size
    };
    this->type = type;
    return error_code::success;
}

error_code buffer::upload_to_gpu()
{
    if (gpu.vk)
        return error_code::success;
    return gpu.upload(cpu.data, cpu.size,
        buffer_type_to_vk_buffer_usage_flags(type),
        buffer_type_to_vma_alloc_create_flags(type));
}

void buffer::unload_from_gpu()
{
    if (!gpu.vk)
        return;
    gpu.destroy();
    gpu.vk = nullptr;
}

void buffer::destroy()
{
    if (gpu.vk)
    {
        gpu.destroy();
        gpu.vk = nullptr;
    }
    delete[] reinterpret_cast<cache_aligned_bytes*>(const_cast<std::byte*>(cpu.data));
    cpu.data = nullptr;
}
}