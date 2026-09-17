#pragma once

#include <veer_core/utils.h>

#include <veer_core/error_code.h>

#include <vulkan/vulkan.h>

#include <cstddef>

class VmaAllocation_T;
namespace ve
{
struct vk_buffer
{
    DECLARE_NO_COPY(vk_buffer);

    vk_buffer(vk_buffer&& other) :
        vk(other.vk),
        alloc(other.alloc)
    {
        other.vk = nullptr;
        other.alloc = nullptr;
    }
    vk_buffer& operator=(vk_buffer&& other)
    {
        vk = other.vk;
        alloc = other.alloc;
        other.vk = nullptr;
        other.alloc = nullptr;
        return *this;
    }

    error_code upload(const std::byte*, VkDeviceSize, VkBufferCreateFlags, VkFlags);
    void destroy();

    ~vk_buffer() = default;

    VkBuffer vk{};
    VmaAllocation_T* alloc{};
};

enum class buffer_type
{
    vertex,
    index,
    vertex_and_index,
    image_transfer_src,
};

struct buffer
{
    DECLARE_NO_COPY(buffer);

    buffer(buffer&& other)
    : cpu(std::move(other.cpu)),
    gpu(std::move(other.gpu)),
    type(other.type)
    {
        other.cpu.invalidate();
    }

    buffer& operator=(buffer&& other)
    {
        cpu = std::move(other.cpu);
        gpu = std::move(other.gpu);
        type = other.type;
        other.cpu.invalidate();
        return *this;
    }

    error_code init(const std::byte*, size_t, buffer_type);
    error_code init(const byte_span&, buffer_type);
    error_code init(size_t, buffer_type);
    void destroy();

    error_code upload_to_gpu();
    void unload_from_gpu();

    ~buffer() = default;

    byte_span cpu{};
    vk_buffer gpu{};
    buffer_type type{};
};
}