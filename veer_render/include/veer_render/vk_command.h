#pragma once

#include "vk_buffer.h"
#include "vk_sync.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <expected>
#include <vector>

namespace ve
{
struct vk_command_buffer : public vk_weak_ptr<VkCommandBuffer>
{
    error_code begin(bool=true);
    error_code end();
    error_code submit(vk_weak_ptr<VkQueue>, vk_weak_ptr<VkFence> = nullptr);

    void copy_buffer(const vk_buffer&, const vk_buffer&, size_t);
    void copy_buffer(const vk_buffer& source, const vk_buffer& target) { copy_buffer(source, target, target.size); }

    void pipeline_barrier(const VkDependencyInfo&);
    void copy_buffer_to_image(const VkCopyBufferToImageInfo2&);

    using parent_type = vk_weak_ptr<VkCommandBuffer>;
    using parent_type::parent_type;
    using parent_type::operator=;
};

struct vk_command_pool : public vk_unique_ptr<VkCommandPool>
{
    error_code init(VkCommandPoolCreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    [[nodiscard]]
    std::expected<vk_weak_ptr<VkCommandBuffer>, error_code> allocate_cmd_buffer(bool is_primary=true) const;
    [[nodiscard]]
    std::expected<std::vector<vk_weak_ptr<VkCommandBuffer>>, error_code> allocate_cmd_buffers(uint32_t, bool is_primary=true) const;
};
}