#include "internal/ktx2.h"

#include "vk_command.h"
#include "vk_context.h"

#include "internal/staging_buffer.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace ve
{
error_code ktx2_texture_wrapper::load(const std::string& path)
{
    if (FAILED(ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &m_ktx)))
        return error(error_code::texture, "Failed to create texture from \"{}\"", path);
    if (ktxTexture2_NeedsTranscoding(m_ktx) && FAILED(ktxTexture2_TranscodeBasis(m_ktx, KTX_TTF_BC7_RGBA, 0)))
        return error(error_code::texture, "Failed to transcode texture to suitable format");
    return error_code::success;
}

error_code ktx2_texture_wrapper::initialize_texture(ve::texture& new_tex)
{
    SAFE_JUST_INIT(new_tex, m_ktx);

    auto& staging = staging_buffer::get();
    staging.wait_until_finished_transfering();
    SAFE_CALL(staging.resize_if_needed(m_ktx->dataSize));
    staging.copy_to_mapped(reinterpret_cast<void*>(m_ktx->pData), m_ktx->dataSize);

    vk_command_buffer SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE(cmd_buf, vk_context::get().allocate_cmd_buffer());
    {
        SAFE_CALL(cmd_buf.begin());
        
        VkImageMemoryBarrier2 barrier_tex_image{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .image = new_tex,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_metadata.mip_levels, .layerCount = 1 }
        };

        VkDependencyInfo barrier_tex_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier_tex_image
        };
        vkCmdPipelineBarrier2(cmd_buf, &barrier_tex_info);

        std::vector<VkBufferImageCopy2> copy_regions{};
        for (uint8_t i = 0; i < new_tex.m_metadata.mip_levels; ++i)
        {
            size_t mip_offset{};
            if (FAILED(ktxTexture2_GetImageOffset(m_ktx, i, 0, 0, &mip_offset)))
                return error(error_code::texture, "Failed to get image mipmap offset");
            copy_regions.push_back({
                .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
                .bufferOffset = mip_offset,
                .imageSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .layerCount = 1 },
                .imageExtent{
                    .width = static_cast<uint32_t>(new_tex.m_metadata.width >> i),
                    .height = static_cast<uint32_t>(new_tex.m_metadata.height >> i),
                    .depth = 1
                }
            });
        }

        VkCopyBufferToImageInfo2 copy_buf_to_image_info{
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .srcBuffer = staging,
            .dstImage = new_tex,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = static_cast<uint32_t>(copy_regions.size()),
            .pRegions = copy_regions.data()
        };
        vkCmdCopyBufferToImage2(cmd_buf, &copy_buf_to_image_info);

        VkImageMemoryBarrier2 barrier_tex_read{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .image = new_tex,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_metadata.mip_levels, .layerCount = 1 }
        };
        barrier_tex_info.pImageMemoryBarriers = &barrier_tex_read;
        vkCmdPipelineBarrier2(cmd_buf, &barrier_tex_info);

        SAFE_CALL(cmd_buf.end());
    }
    staging.fence.reset();
    SAFE_CALL(cmd_buf.submit(vk_context::get().queue, staging.fence));

    return error_code::success;
}

ktx2_texture_wrapper::~ktx2_texture_wrapper()
{
    ktxTexture2_Destroy(m_ktx);
}
}