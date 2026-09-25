#pragma once

#include "render_data.h"
#include "vk_command.h"
#include "vk_sync.h"
#include "vk_ptr.h"

#include <veer_core/utils.h>

#include <vulkan/vulkan.h>

#include <array>
#include <cassert>
#include <unordered_map>

struct VmaAllocator_T;
namespace ve
{
static constexpr uint8_t frames_in_flight = 2;

struct vk_instance : public vk_unique_ptr<VkInstance>
{
    error_code init();
    void destroy();
};

struct vk_physical_device : public vk_weak_ptr<VkPhysicalDevice>
{
    error_code init(VkPhysicalDevice);

    std::string name{};
};

struct vk_device : public vk_unique_ptr<VkDevice>
{
    error_code init();

    void wait_idle() noexcept { vkDeviceWaitIdle(this->vk); }
};

struct vk_queue : public vk_weak_ptr<VkQueue>
{
    error_code init();

    uint32_t family{};
};

struct vk_allocator : public vk_unique_ptr<VmaAllocator_T*>
{
    error_code init();
};

struct vk_descriptor_set_layout : public vk_unique_ptr<VkDescriptorSetLayout>
{
    error_code init();
};

struct vk_descriptor_pool : public vk_unique_ptr<VkDescriptorPool>
{
    error_code init(size_t=1);

    [[nodiscard]] std::expected<vk_weak_ptr<VkDescriptorSet>, error_code> allocate_descriptor_set(vk_weak_ptr<VkDescriptorSetLayout>);
    [[nodiscard]] std::expected<std::vector<vk_weak_ptr<VkDescriptorSet>>, error_code> allocate_descriptor_sets(vk_weak_ptr<VkDescriptorSetLayout>, size_t=1);
};

struct vk_frame_context
{
    VEER_DECLARE_NO_COPY_NO_MOVE(vk_frame_context);

    error_code init();
    void destroy();

    void commit();

    render_data data{};
    vk_command_pool pool{};
    // TODO: command buffers will need to be somewhere else, but will do for now
    vk_command_buffer command_buffer{};
    vk_fence render_start_fence{};
    vk_semaphore image_acquired_semaphore{};
};

struct vk_context
{
    VEER_DECLARE_NO_COPY_NO_MOVE(vk_context);

    error_code init();
    void destroy();

    vk_physical_device& gpu() noexcept { assert(gpu_index < gpus.size()); return gpus[gpu_index]; }
    const vk_physical_device& gpu() const noexcept { assert(gpu_index < gpus.size()); return gpus[gpu_index]; }

    void advance_frame() noexcept { current_frame_index = 1 - current_frame_index; }

    vk_frame_context& current_frame() noexcept { return frames[current_frame_index]; }
    const vk_frame_context& current_frame() const noexcept { return frames[current_frame_index]; }

    const vk_frame_context& prev_frame() const noexcept { return frames[1 - current_frame_index]; }
    const vk_frame_context& next_frame() const noexcept { return frames[1 - current_frame_index]; }

    [[nodiscard]] decltype(auto) allocate_cmd_buffer(bool is_primary=true) const { return current_frame().pool.allocate_cmd_buffer(is_primary); }
    [[nodiscard]] decltype(auto) allocate_cmd_buffers(uint32_t n, bool is_primary=true) const { return current_frame().pool.allocate_cmd_buffers(n, is_primary); }

    vk_instance instance{};
    std::vector<vk_physical_device> gpus{};
    vk_device device{};
    vk_queue queue{};
    vk_allocator allocator{};
    vk_descriptor_set_layout layout{};
    vk_descriptor_pool desc_pool{};
    std::array<vk_frame_context, frames_in_flight> frames{};
    uint8_t gpu_index{};
    uint8_t current_frame_index{};

    _VEER_SINGLETON(vk_context);

private:
    error_code select_gpu();
};
}
