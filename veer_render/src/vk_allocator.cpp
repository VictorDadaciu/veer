#include "vk_allocator.h"

#include "vk_context.h"

#include <veer_core/log.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace ve
{
error_code vk_allocator::init()
{
    VmaVulkanFunctions vk_funcs{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocator_create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, 
        .physicalDevice = context.physical_device.vk,
        .device = context.device.vk,
        .pVulkanFunctions = &vk_funcs,
        .instance = context.instance.vk
    };

    if (FAILED(vmaCreateAllocator(&allocator_create_info, &vk)))
        return error(error_code::initialization, "Failed to initialize VMA allocator");

    return error_code::success;
}

void vk_allocator::destroy()
{
    vmaDestroyAllocator(vk);
}
}
