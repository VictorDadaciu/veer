#pragma once

#include <vulkan/vulkan.h>

struct SDL_Window;
struct VmaAllocator_T;
struct VmaAllocation_T;
namespace ve::vk
{
void destroy(VkInstance);
void destroy(VkDevice);
void destroy(VmaAllocator_T*);
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
void destroy(VkBuffer, VmaAllocation_T*);
void destroy(VkImage, VmaAllocation_T*);
}