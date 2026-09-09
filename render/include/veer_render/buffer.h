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
    vk_buffer() = default;
    vk_buffer(const vk_buffer&) = delete;
    vk_buffer(vk_buffer&& other) :
        vk(other.vk),
        alloc(other.alloc)
    {
        other.vk = nullptr;
        other.alloc = nullptr;
    }
    vk_buffer& operator=(const vk_buffer&) = delete;
    vk_buffer& operator=(vk_buffer&& other)
    {
        vk = other.vk;
        alloc = other.alloc;
        other.vk = nullptr;
        other.alloc = nullptr;
        return *this;
    }

    error_code upload(const std::byte*, VkDeviceSize);
    void destroy();

    ~vk_buffer() = default;

    VkBuffer vk{};
    VmaAllocation_T* alloc{};
};

struct buffer
{
    buffer() = default;
    buffer(const buffer&) = delete;
    buffer(buffer&&) = default;
    buffer& operator=(const buffer&) = delete;
    buffer& operator=(buffer&&) = default;

    error_code init(const std::byte*, size_t);
    error_code init(const byte_span&);
    error_code init(size_t);
    void destroy();

    error_code upload_to_gpu();
    void unload_from_gpu();

    ~buffer() = default;

    byte_span cpu{};
    vk_buffer gpu{};
};
}