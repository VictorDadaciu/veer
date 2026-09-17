#include "asset.h"

#include "vk_context.h"

#include <veer_core/file.h>
#include <veer_core/log.h>

#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include <tinygltf/tiny_gltf_v3.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <vma/vk_mem_alloc.h>

#include <flat_map>
#include <vector>

namespace ve
{
// TODO: move this to internal file
struct gltf_model_wrapper : public tg3_model
{
    VEER_DECLARE_NO_COPY_NO_MOVE(gltf_model_wrapper);

    error_code load(const std::string& path)
    {
        trace("Processing as gltf file...");
        
        tg3_error_stack errs;
        tg3_error_stack_init(&errs);
        if (tg3_parse_file(this, &errs, path.c_str(), path.size(), nullptr) != TG3_OK)
        {
            for (uint32_t i = 0; i < errs.count; ++i)
                warn(errs.entries[i].message ? errs.entries[i].message : "(null)");
            tg3_error_stack_free(&errs);
            return error(error_code::file_read, "Failed to load asset file \"{}\"", path);
        }

        tg3_error_stack_free(&errs);
        return error_code::success;
    }

    size_t get_buffer_and_update_buffers_if_needed(mesh& new_mesh, size_t buffer_index)
    {
        auto it = buffer_indices_map.find(buffer_index);
        if (it != buffer_indices_map.end())
            return it->second;
        
        size_t exact_byte_length = this->buffers[buffer_index].byte_length;
        new_mesh.m_buffer_views.push_back({
            .offset = total_size,
            .size = exact_byte_length
        });
        total_size += next_multiple_of_cache_line_size(exact_byte_length);
        return buffer_indices_map.insert({buffer_index, buffer_indices_map.size()}).first->second;
    }

    error_code allocate_and_copy(mesh& new_mesh)
    {
        try
        {
            SAFE_JUST_INIT(new_mesh.m_buffer, total_size, buffer_type::vertex_and_index);
            for (const auto& index_pairs : buffer_indices_map)
                memcpy(
                    reinterpret_cast<void*>(
                        const_cast<std::byte*>(
                            new_mesh.m_buffer.cpu.data + new_mesh.m_buffer_views[index_pairs.second].offset
                        )
                    ),
                    this->buffers[index_pairs.first].data.data,
                    new_mesh.m_buffer_views[index_pairs.second].size
                );
        }
        catch(...)
        {
            return error_code::allocation;
        }
        return error_code::success;
    }

    error_code initialize_mesh(mesh& new_mesh, size_t mesh_index)
    {
        buffer_indices_map.clear();
        total_size = 0zu;

        const tg3_mesh& mesh_spec = this->meshes[mesh_index];
        new_mesh.m_name = std::string(mesh_spec.name.data, mesh_spec.name.len);
        new_mesh.m_primitives.resize(mesh_spec.primitives_count);
        for (size_t i = 0; i < mesh_spec.primitives_count; ++i)
        {
            const tg3_primitive& primitive_spec = mesh_spec.primitives[i];
            mesh_primitive& new_primitive = new_mesh.m_primitives[i];
            new_primitive.m_parent_index = mesh_index;
            new_primitive.m_render_mode = static_cast<render_mode>(primitive_spec.mode);
            for (size_t j = 0; j < primitive_spec.attributes_count; ++j)
            {
                const tg3_str_int_pair& kv = primitive_spec.attributes[j];
                const tg3_accessor accessor = this->accessors[kv.value];
                const tg3_buffer_view& buffer_view = this->buffer_views[accessor.buffer_view];

                size_t buffer_index = get_buffer_and_update_buffers_if_needed(new_mesh, buffer_view.buffer);

                std::string attribute_name = std::string(kv.key.data, kv.key.len);
                new_primitive.m_vertex_attribute_views[attribute_name] = attribute_view{
                    .buffer = buffer_index,
                    .byte_offset = buffer_view.byte_offset + accessor.byte_offset,
                    .element_count = accessor.count,
                    .stride = buffer_view.byte_stride,
                    .component_size = static_cast<uint8_t>(tg3_component_size(accessor.component_type)),
                    .component_count = static_cast<uint8_t>(tg3_num_components(accessor.type)),
                    .floating_point = accessor.type >= TG3_COMPONENT_TYPE_FLOAT,
                };
            }
            if (primitive_spec.indices == -1)
                continue;

            const tg3_accessor accessor = this->accessors[primitive_spec.indices];
            const tg3_buffer_view& buffer_view = this->buffer_views[accessor.buffer_view];

            size_t buffer_index = get_buffer_and_update_buffers_if_needed(new_mesh, buffer_view.buffer);

            new_primitive.m_index_attribute_view = attribute_view{
                .buffer = buffer_index,
                .byte_offset = buffer_view.byte_offset + accessor.byte_offset,
                .element_count = accessor.count,
                .component_size = static_cast<uint8_t>(tg3_component_size(accessor.component_type)),
                .component_count = 1,
                .floating_point = false,
            };
        }
        return allocate_and_copy(new_mesh);
    }

    ~gltf_model_wrapper() { tg3_model_free(this); }

    std::flat_map<size_t, size_t> buffer_indices_map{};
    size_t total_size{};
};

// TODO: move this to internal file
struct ktx2_texture_wrapper
{
    DECLARE_NO_COPY_NO_MOVE(ktx2_texture_wrapper);

    error_code load(const std::string& path)
    {
        if (FAILED(ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx)))
            return error(error_code::file_read, "Failed to create texture from \"{}\"", path);

        return error_code::success;
    }

    error_code initialize_texture(texture& new_tex)
    {
        new_tex.m_width = ktx->baseWidth;
        new_tex.m_height = ktx->baseHeight;
        new_tex.m_mip_levels = ktx->numLevels;

        VkImageCreateInfo tex_image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = ktxTexture2_GetVkFormat(ktx),
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
        if (FAILED(vmaCreateImage(context.allocator.vk, &tex_image_create_info, &tex_image_alloc_create_info, &new_tex.m_image, &new_tex.m_alloc, nullptr)))
            return error(error_code::allocation, "Failed to allocate texture");

        VkImageViewCreateInfo tex_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = new_tex.m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = tex_image_create_info.format,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = new_tex.m_mip_levels, .layerCount = 1 }
        };
        if (FAILED(vkCreateImageView(context.device.vk, &tex_view_create_info, nullptr, &new_tex.m_image_view)))
            return error(error_code::initialization, "Failed to iniitalize texture's image view");

        SAFE_JUST_INIT(staging_buffer, reinterpret_cast<std::byte*>(ktx->pData), ktx->dataSize, buffer_type::image_transfer_src);
        if (staging_buffer.upload_to_gpu() != error_code::success)
            return error(error_code::allocation, "Failed to allocate staging buffer");

        VkFenceCreateInfo one_shot_fence_create_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        if (FAILED(vkCreateFence(context.device.vk, &one_shot_fence_create_info, nullptr, &one_shot_fence)))
            return error(error_code::initialization, "Failed to create one shot fence");

        VkCommandBuffer SAFE_CALL_HANDLE_EXPECTED(one_shot_cmd_buffer, context.current_frame().pool.allocate_cmd_buffer());
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
                if (FAILED(ktxTexture2_GetImageOffset(ktx, i, 0, 0, &mip_offset)))
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
                .srcBuffer = staging_buffer.gpu.vk,
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

        if (FAILED(vkQueueSubmit2(context.device.queue.vk, 1, &one_time_submit_info, one_shot_fence)))
            return error(error_code::render_submit, "Failed to submit command buffer");

        if (FAILED(vkWaitForFences(context.device.vk, 1, &one_shot_fence, VK_TRUE, UINT64_MAX)))
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
        if (FAILED(vkCreateSampler(context.device.vk, &sampler_create_info, nullptr, &new_tex.m_sampler)))
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
        if (FAILED(vkCreateDescriptorSetLayout(context.device.vk, &desc_layout_tex_create_info, nullptr, &new_tex.m_desc_layout)))
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
        if (FAILED(vkCreateDescriptorPool(context.device.vk, &desc_pool_create_info, nullptr, &new_tex.m_desc_pool)))
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
        if (FAILED(vkAllocateDescriptorSets(context.device.vk, &tex_desc_set_alloc, &new_tex.m_desc_set)))
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
        vkUpdateDescriptorSets(context.device.vk, 1, &write_desc_set, 0, nullptr);

        return error_code::success;
    }

    ~ktx2_texture_wrapper()
    {
        staging_buffer.unload_from_gpu();
        vkDestroyFence(context.device.vk, one_shot_fence, nullptr);
        ktxTexture2_Destroy(ktx);
    }

    ktxTexture2* ktx{};
    buffer staging_buffer{};
    // TODO: not with fence, just work with a placeholder until it's done
    VkFence one_shot_fence{};
};

// TODO: move this to internal file
struct asset_manager
{
    VEER_DECLARE_NO_COPY(asset_manager);

    ~asset_manager() = default;

    void unload_all()
    {
        unload_back(asset_type::mesh);
        unload_back(asset_type::texture);
    }

    void unload_back(asset_type type, size_t n=0)
    {
        switch (type)
        {
            case asset_type::mesh:
            {
                if (n == 0 || n >= meshes.size())
                {
                    for (auto& mesh : meshes)
                        mesh.destroy();
                    meshes.clear();
                    return;
                }
                size_t i = meshes.size();
                while (n-- > 0)
                {
                    meshes[--i].destroy();
                    meshes.pop_back();
                }
            }
            break;
            case asset_type::texture:
            {
                if (n == 0 || n >= textures.size())
                {
                    for (auto& texture : textures)
                        texture.destroy();
                    textures.clear();
                    return;
                }
                size_t i = textures.size();
                while (n-- > 0)
                {
                    textures[--i].destroy();
                    textures.pop_back();
                }
            }
            break;
            default:
                warn("asset_type with id \"{}\" does not exist", static_cast<size_t>(type));
                assert(false);
                return;
        }
    }
    
    asset_load_return_t load_gltf_file(const std::string& path)
    {
        std::vector<asset_load_metadata> res{};
        {
            gltf_model_wrapper model{};
            SAFE_CALL_RETURN_EXPECTED(model.load(path));
            if (model.meshes_count == 0)
                return std::unexpected(error(error_code::file_read, "Asset file \"{}\" is invalid: has no meshes", path));

            for (size_t i = 0; i < model.meshes_count; ++i)
            {
                mesh& new_mesh = meshes.emplace_back();
                if (model.initialize_mesh(new_mesh, i) != error_code::success)
                {
                    unload_back(asset_type::mesh, i);
                    return std::unexpected(error(error_code::file_read, "Failed to read mesh file \"{}\"", path));
                }

                res.push_back({
                    .name = new_mesh.m_name,
                    .index = meshes.size() - 1,
                    .type = asset_type::mesh,
                });
            }
            info("Successfully loaded {} mesh(es) from \"{}\"", model.meshes_count, path);
        }
        return res;
    }

    asset_load_return_t load_ktx2_file(const std::string& path)
    {
        std::vector<asset_load_metadata> res{};
        {
            ktx2_texture_wrapper tex{};
            SAFE_CALL_RETURN_EXPECTED(tex.load(path));

            texture& new_tex = textures.emplace_back();
            if (tex.initialize_texture(new_tex) != error_code::success)
            {
                unload_back(asset_type::texture, 1);
                return std::unexpected(error(error_code::file_read, "Failed to read texture file \"{}\"", path));
            }

            res.push_back({
                .name = file::stem(path),
                .index = textures.size() - 1,
                .type = asset_type::texture
            });
            info("Successfully loaded texture from \"{}\"", path);
        }
        return res;
    }

    std::vector<mesh> meshes;
    std::vector<texture> textures;
} asset_mgr;
}

namespace ve::assets
{
asset_load_return_t load(const std::string& path)
{
    if (!file::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "Asset file \"{}\" doesn't exist", path));
    std::string extension = file::extension(path);
    info("Loading asset file at \"{}\"...", path);
    if (extension == ".glb" || extension == ".gltf")
    {
        return asset_mgr.load_gltf_file(path);
    }
    else if (extension == ".ktx2")
    {
        return asset_mgr.load_ktx2_file(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid asset file type \"{}\"", path));
    }
}

ve::mesh& mesh(const size_t& index)
{
    return asset_mgr.meshes[index];
}

ve::texture& texture(const size_t& index)
{
    return asset_mgr.textures[index];
}

void unload_all()
{
    asset_mgr.unload_all();
}
}