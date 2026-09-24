#include "vk_swapchain.h"

#include "vk_context.h"
#include "window.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <cstring>

namespace ve
{
error_code vk_swapchain_link::init(vk_weak_ptr<VkImage> swapchain_image)
{
    image = swapchain_image;
    VkImageViewCreateInfo view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
    };
    if (FAILED(vkCreateImageView(vk_context::get().device, &view_create_info, nullptr, image_view.write())))
        return error(error_code::initialization, "Failed to create swapchain image view");

    SAFE_JUST_INIT(render_complete_semaphore);

    return error_code::success;
}

void vk_swapchain_link::destroy()
{
    render_complete_semaphore.destroy();
    image_view.destroy();
}

error_code vk_swapchain::init(const window* win)
{
    extent = win->surface.capabilities.currentExtent;
    if (extent.width == 0xFFFFFFFF)
    {
        size_t w{}, h{};
        if (win->size(w, h) == error_code::window)
            return error(error_code::initialization, "Failed to initialize swapchain");
        extent = {
            .width = static_cast<uint32_t>(w),
            .height = static_cast<uint32_t>(h),
        };
    }

    image_format = VK_FORMAT_B8G8R8A8_SRGB;
    VkSwapchainCreateInfoKHR swapchain_create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = win->surface,
        .minImageCount = win->surface.capabilities.minImageCount,
        .imageFormat = image_format,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{.width = extent.width, .height = extent.height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };

    if (FAILED(vkCreateSwapchainKHR(vk_context::get().device, &swapchain_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create swapchain");

    {
        uint32_t image_count{0};
        if (FAILED(vkGetSwapchainImagesKHR(vk_context::get().device, vk, &image_count, nullptr)))
            return error(error_code::initialization, "Failed to get swapchain images");
        std::vector<VkImage> images(image_count);
        if (FAILED(vkGetSwapchainImagesKHR(vk_context::get().device, vk, &image_count, images.data())))
            return error(error_code::initialization, "Failed to get swapchain images");
        links.resize(image_count);
        for (size_t i = 0; i < image_count; ++i)
            SAFE_JUST_INIT(links[i], images[i]);
    }

    std::vector<VkFormat> depth_format_list{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    depth_format = VK_FORMAT_UNDEFINED;
    for (VkFormat& format : depth_format_list)
    {
        VkFormatProperties2 format_properties{
            .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
        };
        vkGetPhysicalDeviceFormatProperties2(vk_context::get().gpu(), format, &format_properties);
        if (format_properties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depth_format = format;
            break;
        }
    }
    if (depth_format == VK_FORMAT_UNDEFINED)
        return error(error_code::initialization, "Failed to find appropriate depth format");
    
    VkImageCreateInfo depth_image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent{.width = extent.width, .height = extent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo alloc_create_info{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (FAILED(vmaCreateImage(vk_context::get().allocator, &depth_image_create_info, &alloc_create_info, depth_image.write(), depth_image.allocation().write(), nullptr)))
        return error(error_code::allocation, "Failed to create image");
    
    VkImageViewCreateInfo depth_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_format,
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
    };
    if (FAILED(vkCreateImageView(vk_context::get().device, &depth_view_create_info, nullptr, depth_image.view.write())))
        return error(error_code::initialization, "Failed to create swapchain depth image view");

    return error_code::success;
}

void vk_swapchain::destroy()
{
    depth_image.destroy();

    for (auto& link : links)
        link.destroy();

    vk_unique_ptr<VkSwapchainKHR>::destroy();
}
}
