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
    new_tex.metadata.width = m_ktx->baseWidth;
    new_tex.metadata.height = m_ktx->baseHeight;
    new_tex.metadata.size = m_ktx->dataSize;
    new_tex.metadata.format = ktxTexture2_GetVkFormat(m_ktx);
    new_tex.metadata.mip_levels = m_ktx->numLevels;
    
    VkImageCreateInfo tex_image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = new_tex.metadata.format,
        .extent = { 
            .width = static_cast<uint32_t>(new_tex.metadata.width),
            .height = static_cast<uint32_t>(new_tex.metadata.height),
            .depth = 1
        },
        .mipLevels = new_tex.metadata.mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };
    VmaAllocationCreateInfo tex_image_alloc_create_info{
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    if (FAILED(vmaCreateImage(vk_context::get().allocator, &tex_image_create_info, &tex_image_alloc_create_info, new_tex.write(), new_tex.allocation().write(), nullptr)))
        return error(error_code::allocation, "Failed to allocate texture");

    auto& staging = staging_buffer::get();
    staging.wait_until_finished_transfering();
    SAFE_CALL(staging.resize_if_needed(m_ktx->dataSize));
    staging.copy_to_mapped(reinterpret_cast<void*>(m_ktx->pData), m_ktx->dataSize);
    
    std::vector<VkBufferImageCopy2> copy_regions{};
    for (uint8_t i = 0; i < new_tex.metadata.mip_levels; ++i)
    {
        size_t mip_offset{};
        if (FAILED(ktxTexture2_GetImageOffset(m_ktx, i, 0, 0, &mip_offset)))
            return error(error_code::texture, "Failed to get image mipmap offset");
        copy_regions.push_back({
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .bufferOffset = mip_offset,
            .imageSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .layerCount = 1 },
            .imageExtent{
                .width = static_cast<uint32_t>(new_tex.metadata.width >> i),
                .height = static_cast<uint32_t>(new_tex.metadata.height >> i),
                .depth = 1
            }
        });
    }
    SAFE_CALL(staging.transfer_to_image(new_tex, copy_regions));

    return new_tex.init();
}

ktx2_texture_wrapper::~ktx2_texture_wrapper()
{
    ktxTexture2_Destroy(m_ktx);
}
}