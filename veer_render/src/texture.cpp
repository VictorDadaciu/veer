#include "texture.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <cassert>

namespace ve
{
error_code texture::init()
{
    VkImageViewCreateInfo tex_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vk,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = metadata.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = metadata.mip_levels, .layerCount = 1 }
    };
    if (FAILED(vkCreateImageView(vk_context::get().device, &tex_view_create_info, nullptr, view.write())))
        return error(error_code::initialization, "Failed to iniitalize texture's image view");

    VkSamplerCreateInfo sampler_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .maxLod = static_cast<float>(metadata.mip_levels)
    };
    if (FAILED(vkCreateSampler(vk_context::get().device, &sampler_create_info, nullptr, sampler.write())))
        return error(error_code::initialization, "Failed to create texture sampler");

    SAFE_CALL_HANDLE_EXPECTED(descriptor, vk_context::get().desc_pool.allocate_descriptor_set(vk_context::get().layout));

    VkDescriptorImageInfo desc_image_info{
        .sampler = sampler,
        .imageView = view,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet write_desc_set{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
        .pImageInfo = &desc_image_info
    };
    vkUpdateDescriptorSets(vk_context::get().device, 1, &write_desc_set, 0, nullptr);

    return error_code::success;
}
}