#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

struct SDL_Window;
namespace ve::vk
{
static constexpr uint8_t frames_in_flight = 2;

inline VkExtent2D to_extent_2d(const glm::uvec2& vec) noexcept
{
    return VkExtent2D{
        .width = vec.x,
        .height = vec.y
    };
}

void destroy(VkInstance);
void destroy(VkDevice);
void destroy(VmaAllocator);
void destroy(VkDescriptorSetLayout);
void destroy(VkDescriptorPool);
void destroy(VkImageView);
void destroy(VkSurfaceKHR);
void destroy(VkSwapchainKHR);
void destroy(VkFence);
void destroy(VkSemaphore);
void destroy(SDL_Window*);
void destroy(VkSampler);
void destroy(VkCommandPool);
void destroy(VkBuffer, VmaAllocation);
void destroy(VkImage, VmaAllocation);
void destroy(VkShaderModule);
void destroy(VkPipelineLayout);
void destroy(VkPipeline);
}