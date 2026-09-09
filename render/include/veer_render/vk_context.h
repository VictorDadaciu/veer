#pragma once

#include "vk_instance.h"
#include "vk_device.h"
#include "vk_queue.h"
#include "vk_allocator.h"

#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <array>

namespace ve
{
static constexpr uint8_t frames_in_flight = 2;

struct vk_frame_context
{
    DECLARE_SINGLE_COPY(vk_frame_context);

    error_code init();
    void destroy();

    ~vk_frame_context() = default;

    VkCommandPool command_pool{};
    VkCommandBuffer command_buffer{};
    VkFence render_start_fence{};
    VkSemaphore image_acquired_semaphore{};
};

extern struct vk_context
{
    DECLARE_SINGLE_COPY(vk_context);

    error_code init();
    void destroy();

    ~vk_context() = default;

    void advance_frame() noexcept { current_frame_index = 1 - current_frame_index; }

    vk_frame_context& current_frame() noexcept { return frames[current_frame_index]; }
    const vk_frame_context& current_frame() const noexcept { return frames[current_frame_index]; }

    const vk_frame_context& prev_frame() const noexcept { return frames[1 - current_frame_index]; }
    const vk_frame_context& next_frame() const noexcept { return frames[1 - current_frame_index]; }

    vk_instance instance{};
    vk_physical_device physical_device{};
    vk_device device{};
    vk_allocator allocator{};
    std::array<vk_frame_context, frames_in_flight> frames{};
    uint8_t current_frame_index{};
} context;
}

