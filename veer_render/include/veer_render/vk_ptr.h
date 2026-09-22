#pragma once

#include "vk_utils.h"

#include <type_traits>
#include <utility>

struct VmaAllocation_T;
namespace ve
{
template<typename vk_res>
    requires (std::is_pointer_v<vk_res>)
class vk_weak_ptr
{
public:
    vk_weak_ptr(vk_res ptr = nullptr) noexcept : vk(ptr) {}
    vk_weak_ptr(const vk_weak_ptr& other) noexcept : vk(other.vk) {}
    vk_weak_ptr(vk_weak_ptr&& other) noexcept : vk(other.vk) { other.vk = nullptr; }

    vk_weak_ptr& operator=(vk_res ptr) noexcept
    {
        this->vk = ptr;
        return *this;
    }

    vk_weak_ptr& operator=(const vk_weak_ptr& other) noexcept
    {
        this->vk = other.vk;
        return *this;
    }

    vk_weak_ptr& operator=(vk_weak_ptr&& other) noexcept
    {
        this->vk = other.vk;
        other.vk = nullptr;
        return *this;
    }

    ~vk_weak_ptr() = default;

    operator vk_res() noexcept { return this->vk; }
    operator vk_res() const noexcept { return this->vk; }

    vk_res operator*() noexcept { return this->vk; }
    vk_res operator*() const noexcept { return this->vk; }

    vk_res* write() noexcept { return &this->vk; }

protected:
    vk_res vk{};
};

template<typename vk_res>
    requires (std::is_pointer_v<vk_res>)
class vk_unique_ptr : public vk_weak_ptr<vk_res>
{
public:
    vk_unique_ptr(vk_res ptr=nullptr) noexcept : vk_weak_ptr<vk_res>(ptr) {}
    vk_unique_ptr(const vk_weak_ptr<vk_res>& other) = delete;
    vk_unique_ptr(vk_weak_ptr<vk_res>&& other) noexcept : vk_weak_ptr<vk_res>(std::move(other)) {}

    vk_unique_ptr& operator=(vk_res ptr) = delete;
    vk_unique_ptr& operator=(const vk_weak_ptr<vk_res>& other) = delete;

    vk_unique_ptr& operator=(vk_weak_ptr<vk_res>&& other) noexcept
    {
        vk_weak_ptr<vk_res>::operator=(std::move(other));
        return *this;
    }

    void destroy()
    {
        if (!this->vk) return;
        vk::destroy(this->vk);
        this->vk = nullptr;
    }

    ~vk_unique_ptr() { if (this->vk) destroy(); }
};

template<typename vk_res>
    requires (std::is_pointer_v<vk_res>)
class vk_allocated_ptr : public vk_weak_ptr<vk_res>
{
public:
    vk_allocated_ptr(vk_res ptr=nullptr, VmaAllocation_T* allocation=nullptr) noexcept : vk_weak_ptr<vk_res>(ptr), alloc(allocation) {}
    vk_allocated_ptr(const vk_weak_ptr<vk_res>& other) = delete;
    vk_allocated_ptr(vk_weak_ptr<vk_res>&& other) = delete;
    vk_allocated_ptr(vk_allocated_ptr<vk_res>&& other) noexcept : vk_weak_ptr<vk_res>(std::move(other)), alloc(std::move(other.alloc)) {}

    vk_allocated_ptr& operator=(vk_res ptr) = delete;
    vk_allocated_ptr& operator=(const vk_weak_ptr<vk_res>& other) = delete;
    vk_allocated_ptr& operator=(vk_weak_ptr<vk_res>&& other) = delete;

    vk_allocated_ptr& operator=(vk_allocated_ptr<vk_res>&& other) noexcept
    {
        vk_weak_ptr<vk_res>::operator=(std::move(other));
        alloc = std::move(other.alloc);
        return *this;
    }

    vk_weak_ptr<VmaAllocation_T*>& allocation() noexcept { return alloc; }
    const vk_weak_ptr<VmaAllocation_T*>& allocation() const noexcept { return alloc; }

    void destroy()
    {
        if (!this->vk) return;
        vk::destroy(this->vk, alloc);
        this->vk = nullptr;
        alloc = nullptr;
    }

    ~vk_allocated_ptr() { if (this->vk) destroy(); }

protected:
    vk_weak_ptr<VmaAllocation_T*> alloc{};
};
}