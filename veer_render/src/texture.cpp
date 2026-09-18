#include "texture.h"

#include "vk_context.h"

#include <vma/vk_mem_alloc.h>

namespace ve
{
void texture::destroy()
{
    vkDestroyDescriptorPool(vk::context().device.vk, m_desc_pool, nullptr);
    vkDestroyDescriptorSetLayout(vk::context().device.vk, m_desc_layout, nullptr);
    vkDestroySampler(vk::context().device.vk, m_sampler, nullptr);
    vkDestroyImageView(vk::context().device.vk, m_image_view, nullptr);
    vmaDestroyImage(vk::context().allocator.vk, m_image, m_alloc);
}
}