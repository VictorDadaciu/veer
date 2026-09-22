#include "texture.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <cassert>

namespace ve
{
error_code texture::init(ktxTexture2* ktx)
{
    assert(ktx);
    m_metadata.width = ktx->baseWidth;
    m_metadata.height = ktx->baseHeight;
    m_metadata.mip_levels = ktx->numLevels;
    
    VkFormat format = ktxTexture2_GetVkFormat(ktx);
    VkImageCreateInfo tex_image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = { .width = static_cast<uint32_t>(m_metadata.width), .height = static_cast<uint32_t>(m_metadata.height), .depth = 1},
        .mipLevels = m_metadata.mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VmaAllocationCreateInfo tex_image_alloc_create_info{
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (FAILED(vmaCreateImage(vk_context::get().allocator, &tex_image_create_info, &tex_image_alloc_create_info, &vk, alloc.write(), nullptr)))
        return error(error_code::allocation, "Failed to allocate texture");

    // TODO: transfer::to_image(m_image, metadata);

    VkImageViewCreateInfo tex_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = vk,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = tex_image_create_info.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = m_metadata.mip_levels, .layerCount = 1 }
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
        .maxLod = static_cast<float>(m_metadata.mip_levels)
    };
    if (FAILED(vkCreateSampler(vk_context::get().device, &sampler_create_info, nullptr, sampler.write())))
        return error(error_code::initialization, "Failed to create texture sampler");

    SAFE_CALL_HANDLE_EXPECTED(m_desc_set, vk_context::get().desc_pool.allocate_descriptor_set(vk_context::get().layout));

    VkDescriptorImageInfo desc_image_info{
        .sampler = sampler,
        .imageView = view,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet write_desc_set{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_desc_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
        .pImageInfo = &desc_image_info
    };
    vkUpdateDescriptorSets(vk_context::get().device, 1, &write_desc_set, 0, nullptr);

    return error_code::success;
}
}