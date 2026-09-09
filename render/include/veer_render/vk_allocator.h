#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

struct VmaAllocator_T;
namespace ve
{
struct vk_allocator
{
    DECLARE_SINGLE_COPY(vk_allocator);

    error_code init();
    void destroy();

    ~vk_allocator() = default;

    VmaAllocator_T* vk{};
};
}
