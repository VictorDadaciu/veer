#include "render_data.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

namespace ve
{
error_code render_data::init()
{
    ubo.size = sizeof(shader_data);
    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = ubo.size,
        .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    };
    VmaAllocationCreateInfo buffer_alloc_create_info{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo alloc_info{};
    if (FAILED(vmaCreateBuffer(vk_context::get().allocator,&buffer_create_info, &buffer_alloc_create_info, ubo.write(), ubo.allocation().write(), &alloc_info)))
        return error(error_code::allocation, "Failed to reallocate staging buffer");
    ubo.mapped = reinterpret_cast<std::byte*>(alloc_info.pMappedData);

    VkBufferDeviceAddressInfo ubo_bda_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = ubo
    };
    device_address = vkGetBufferDeviceAddress(vk_context::get().device, &ubo_bda_info);
    return error_code::success;
}

void render_data::destroy()
{
    ubo.destroy();
}
}