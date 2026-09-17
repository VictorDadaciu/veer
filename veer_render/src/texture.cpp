#include "texture.h"

#include "vk_context.h"

#include <vma/vk_mem_alloc.h>

namespace ve
{
void texture::destroy()
{
    vkDestroyDescriptorPool(context.device.vk, m_desc_pool, nullptr);
    vkDestroyDescriptorSetLayout(context.device.vk, m_desc_layout, nullptr);
    vkDestroySampler(context.device.vk, m_sampler, nullptr);
    vkDestroyImageView(context.device.vk, m_image_view, nullptr);
    vmaDestroyImage(context.allocator.vk, m_image, m_alloc);
}
}