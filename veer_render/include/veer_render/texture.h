#pragma once

#include "vk_image.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <cstdint>

struct VmaAllocation_T;
struct ktxTexture2;
namespace ve
{
struct asset_manager;
class ktx2_texture_wrapper;

struct image_metadata
{
    size_t width{};
    size_t height{};
    size_t size{};
    uint8_t mip_levels{};
};

class texture : public vk_sampled_image
{
public:
    size_t width() const noexcept { return m_metadata.width; }
    size_t height() const noexcept { return m_metadata.height; }
    uint8_t mip_levels() const noexcept { return m_metadata.mip_levels; }
    vk_weak_ptr<VkDescriptorSet> descriptor_set() const noexcept { return m_desc_set; }

private:
    friend struct ve::asset_manager;
    friend class ve::ktx2_texture_wrapper;

    error_code init(ktxTexture2*);
    using vk_sampled_image::destroy;

    vk_weak_ptr<VkDescriptorSet> m_desc_set{};

    image_metadata m_metadata{};
};
}