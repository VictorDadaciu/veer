#pragma once

#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ve::assets
{
struct asset_manager;
class ktx2_texture_wrapper;
}

struct VmaAllocation_T;
namespace ve
{
class texture
{
public:
    VEER_DECLARE_NO_COPY(texture);

    texture(texture&& other)
    : m_image(other.m_image),
    m_image_view(other.m_image_view),
    m_alloc(other.m_alloc),
    m_sampler(other.m_sampler),
    m_desc_layout(other.m_desc_layout),
    m_desc_pool(other.m_desc_pool),
    m_desc_set(other.m_desc_set),
    m_width(other.m_width),
    m_height(other.m_height),
    m_mip_levels(other.m_mip_levels)
    {
        other.m_image = nullptr;
        other.m_image_view = nullptr;
        other.m_alloc = nullptr;
        other.m_sampler = nullptr;
        other.m_desc_layout = nullptr;
        other.m_desc_pool = nullptr;
        other.m_desc_set = nullptr;
    }

    texture& operator=(texture&& other)
    {
        m_image = other.m_image;
        m_image_view = other.m_image_view;
        m_alloc = other.m_alloc;
        m_sampler = other.m_sampler;
        m_desc_layout = other.m_desc_layout;
        m_desc_pool = other.m_desc_pool;
        m_desc_set = other.m_desc_set;
        m_width = other.m_width;
        m_height = other.m_height;
        m_mip_levels = other.m_mip_levels;
        other.m_image = nullptr;
        other.m_image_view = nullptr;
        other.m_alloc = nullptr;
        other.m_sampler = nullptr;
        other.m_desc_layout = nullptr;
        other.m_desc_pool = nullptr;
        other.m_desc_set = nullptr;
        return *this;
    }

    size_t width() const noexcept { return m_width; }
    size_t height() const noexcept { return m_height; }
    uint8_t mip_levels() const noexcept { return m_mip_levels; }

private:
    friend struct ve::assets::asset_manager;
    friend class ve::assets::ktx2_texture_wrapper;

    void destroy();

    // TODO: texture class
    VkImage m_image{};
    VkImageView m_image_view{};
    VmaAllocation_T* m_alloc{};
    VkSampler m_sampler{};
    // TODO: not here
    VkDescriptorSetLayout m_desc_layout{};
    VkDescriptorPool m_desc_pool{};
    VkDescriptorSet m_desc_set{};

    size_t m_width{};
    size_t m_height{};
    uint8_t m_mip_levels{};
};
}