#pragma once

#include "vk_image.h"
#include "vk_buffer.h"
#include "vk_sync.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <vma/vk_mem_alloc.h>

#include <vector>

namespace ve
{
// TODO: expose, make thread-safe etc.
struct staging_buffer : public vk_buffer
{
    error_code init(size_t);
    error_code resize_if_needed(size_t);
    void destroy();

    void copy_to_mapped(const void*, size_t, size_t=0zu);

    error_code transfer_to_buffer(const vk_buffer&);
    error_code transfer_to_image(const vk_image&, const std::vector<VkBufferImageCopy2>&);

    void wait_until_finished_transfering() noexcept;
    bool is_transfering() const noexcept;

    std::byte* mapped{};
    vk_fence fence{};
    
    _VEER_SINGLETON(staging_buffer);
};
}