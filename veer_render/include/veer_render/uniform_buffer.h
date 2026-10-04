#pragma once

#include "vk_buffer.h"
#include "vk_context.h"

#include <veer_core/error_code.h>
#include <veer_core/log.h>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <cassert>

namespace ve
{
template<typename resource_t>
struct uniform_buffer : public persistently_mapped_buffer
{
    error_code init(size_t count=1024)
    {
        size = sizeof(resource_t) * count;
        VkBufferCreateInfo buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        VmaAllocationCreateInfo buffer_alloc_create_info{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
        VmaAllocationInfo alloc_info{};
        if (LEGACY_FAILED(vmaCreateBuffer(vk_context::get().allocator,&buffer_create_info, &buffer_alloc_create_info, &vk, alloc.write(), &alloc_info)))
            return error(error_code::allocation, "Failed to reallocate staging buffer");
        mapped = reinterpret_cast<std::byte*>(alloc_info.pMappedData);

        VkBufferDeviceAddressInfo ubo_bda_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = *this
        };
        address = vkGetBufferDeviceAddress(vk_context::get().device, &ubo_bda_info);
        return error_code::success;
    }

    void resize(size_t n)
    {
        data.resize(n);
    }

    resource_t& operator[](size_t i)
    {
        assert(i < data.size());
        return data[i];
    }

    void commit()
    {
        memcpy(mapped, data.data(), data.size() * sizeof(resource_t));
    }

    void destroy()
    {
        data.clear();
        persistently_mapped_buffer::destroy();
    }

    std::vector<resource_t> data{};
    VkDeviceAddress address{};
};
}