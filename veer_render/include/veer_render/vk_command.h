#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <expected>
#include <vector>

namespace ve
{
struct vk_command_pool
{
    VEER_DECLARE_NO_COPY_NO_MOVE(vk_command_pool);

    error_code init(VkCommandPoolCreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    void destroy();

    ~vk_command_pool() = default;

    std::expected<VkCommandBuffer, error_code> allocate_cmd_buffer(bool is_primary=true) const;
    std::expected<std::vector<VkCommandBuffer>, error_code> allocate_cmd_buffers(bool is_primary, uint32_t) const;

    VkCommandPool vk{};
};
}