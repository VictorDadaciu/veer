#include "internal/ktx2.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace ve::assets
{
error_code ktx2_texture_wrapper::load(const std::string& path)
{
    if (FAILED(ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &m_ktx)))
        return error(error_code::file_read, "Failed to create texture from \"{}\"", path);

    return error_code::success;
}

error_code ktx2_texture_wrapper::initialize_texture(ve::texture& new_tex)
{
    new_tex.m_width = m_ktx->baseWidth;
    new_tex.m_height = m_ktx->baseHeight;
    new_tex.m_mip_levels = m_ktx->numLevels;

    VkImageCreateInfo tex_image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = ktxTexture2_GetVkFormat(m_ktx),
        .extent = { .width = static_cast<uint32_t>(new_tex.m_width), .height = static_cast<uint32_t>(new_tex.m_height), .depth = 1},
        .mipLevels = new_tex.m_mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VmaAllocationCreateInfo tex_image_alloc_create_info{
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (FAILED(vmaCreateImage(vk::context().allocator.vk, &tex_image_create_info, &tex_image_alloc_create_info, &new_tex.m_image, &new_tex.m_alloc, nullptr)))
        return error(error_code::allocation, "Failed to allocate texture");

    VkImageViewCreateInfo tex_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = new_tex.m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = tex_image_create_info.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_mip_levels, .layerCount = 1 }
    };
    if (FAILED(vkCreateImageView(vk::context().device.vk, &tex_view_create_info, nullptr, &new_tex.m_image_view)))
        return error(error_code::initialization, "Failed to iniitalize texture's image view");

    SAFE_JUST_INIT(m_staging_buffer, reinterpret_cast<std::byte*>(m_ktx->pData), m_ktx->dataSize, buffer_type::image_transfer_src);
    if (m_staging_buffer.upload_to_gpu() != error_code::success)
        return error(error_code::allocation, "Failed to allocate staging buffer");

    VkFenceCreateInfo m_one_shot_fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    if (FAILED(vkCreateFence(vk::context().device.vk, &m_one_shot_fence_create_info, nullptr, &m_one_shot_fence)))
        return error(error_code::initialization, "Failed to create one shot fence");

    VkCommandBuffer SAFE_CALL_HANDLE_EXPECTED_INIT(one_shot_cmd_buffer, vk::context().current_frame().pool.allocate_cmd_buffer());
    {
        VkCommandBufferBeginInfo cmd_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        if (FAILED(vkBeginCommandBuffer(one_shot_cmd_buffer, &cmd_buffer_begin_info)))
            return error(error_code::command_record, "Failed to begin command buffer recording");
        
        VkImageMemoryBarrier2 barrier_tex_image{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = new_tex.m_image,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_mip_levels, .layerCount = 1 }
        };

        VkDependencyInfo barrier_tex_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier_tex_image
        };
        vkCmdPipelineBarrier2(one_shot_cmd_buffer, &barrier_tex_info);

        std::vector<VkBufferImageCopy2> copy_regions{};
        for (uint8_t i = 0; i < new_tex.m_mip_levels; ++i)
        {
            size_t mip_offset{};
            if (FAILED(ktxTexture2_GetImageOffset(m_ktx, i, 0, 0, &mip_offset)))
                return error(error_code::texture, "Failed to get image mipmap offset");
            copy_regions.push_back({
                .bufferOffset = mip_offset,
                .imageSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .layerCount = 1 },
                .imageExtent{
                    .width = static_cast<uint32_t>(new_tex.m_width >> i),
                    .height = static_cast<uint32_t>(new_tex.m_height >> i),
                    .depth = 1
                }
            });
        }

        VkCopyBufferToImageInfo2 copy_buf_to_image_info{
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .srcBuffer = m_staging_buffer.gpu.vk,
            .dstImage = new_tex.m_image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = static_cast<uint32_t>(copy_regions.size()),
            .pRegions = copy_regions.data()
        };
        vkCmdCopyBufferToImage2(one_shot_cmd_buffer, &copy_buf_to_image_info);

        VkImageMemoryBarrier2 barrier_tex_read{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .image = new_tex.m_image,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_mip_levels, .layerCount = 1 }
        };
        barrier_tex_info.pImageMemoryBarriers = &barrier_tex_read;
        vkCmdPipelineBarrier2(one_shot_cmd_buffer, &barrier_tex_info);

        if (FAILED(vkEndCommandBuffer(one_shot_cmd_buffer)))
            return error(error_code::command_record, "Failed to end command buffer recording");
    }
    VkCommandBufferSubmitInfo cmd_buffer_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = one_shot_cmd_buffer
    };
    VkSubmitInfo2 one_time_submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_buffer_submit_info,
    };

    if (FAILED(vkQueueSubmit2(vk::context().device.queue.vk, 1, &one_time_submit_info, m_one_shot_fence)))
        return error(error_code::render_submit, "Failed to submit command buffer");

    if (FAILED(vkWaitForFences(vk::context().device.vk, 1, &m_one_shot_fence, VK_TRUE, UINT64_MAX)))
        return error(error_code::synchronization, "Failed to wait for fence");

    VkSamplerCreateInfo sampler_create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .maxLod = static_cast<float>(new_tex.m_mip_levels)
    };
    if (FAILED(vkCreateSampler(vk::context().device.vk, &sampler_create_info, nullptr, &new_tex.m_sampler)))
        return error(error_code::initialization, "Failed to create texture sampler");
    
    // TODO: handle these descriptor objects correctly
    VkDescriptorBindingFlags desc_variable_flag{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
    VkDescriptorSetLayoutBindingFlagsCreateInfo desc_binding_flags{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &desc_variable_flag
    };
    VkDescriptorSetLayoutBinding desc_layout_binding_tex{
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutCreateInfo desc_layout_tex_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &desc_binding_flags,
        .bindingCount = 1,
        .pBindings = &desc_layout_binding_tex
    };
    if (FAILED(vkCreateDescriptorSetLayout(vk::context().device.vk, &desc_layout_tex_create_info, nullptr, &new_tex.m_desc_layout)))
        return error(error_code::initialization, "Failed to create descriptor set layout");

    VkDescriptorPoolSize pool_size{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1
    };
    VkDescriptorPoolCreateInfo desc_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };
    if (FAILED(vkCreateDescriptorPool(vk::context().device.vk, &desc_pool_create_info, nullptr, &new_tex.m_desc_pool)))
        return error(error_code::initialization, "Failed to create descriptor pool");

    uint32_t variable_desc_count = 1;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_desc_count_alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &variable_desc_count
    };
    VkDescriptorSetAllocateInfo tex_desc_set_alloc{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variable_desc_count_alloc_info,
        .descriptorPool = new_tex.m_desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &new_tex.m_desc_layout
    };
    if (FAILED(vkAllocateDescriptorSets(vk::context().device.vk, &tex_desc_set_alloc, &new_tex.m_desc_set)))
        return error(error_code::allocation, "Failed to allocate descriptor sets");

    VkDescriptorImageInfo desc_image_info{
        .sampler = new_tex.m_sampler,
        .imageView = new_tex.m_image_view,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet write_desc_set{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = new_tex.m_desc_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
        .pImageInfo = &desc_image_info
    };
    vkUpdateDescriptorSets(vk::context().device.vk, 1, &write_desc_set, 0, nullptr);

    return error_code::success;
}

ktx2_texture_wrapper::~ktx2_texture_wrapper()
{
    m_staging_buffer.unload_from_gpu();
    vkDestroyFence(vk::context().device.vk, m_one_shot_fence, nullptr);
    ktxTexture2_Destroy(m_ktx);
}
}