#include "vk_utils.h"

#include "vk_context.h"

#include <veer_core/log.h>

#include <vma/vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace ve::vk
{
void destroy(VkInstance instance) { vkDestroyInstance(instance, nullptr); }
void destroy(VkDevice device) { vkDestroyDevice(device, nullptr); }
void destroy(VmaAllocator_T* allocator) { vmaDestroyAllocator(allocator); }
void destroy(VkDescriptorSetLayout layout) { vkDestroyDescriptorSetLayout(vk_context::get().device, layout, nullptr); }
void destroy(VkDescriptorPool pool) { vkDestroyDescriptorPool(vk_context::get().device, pool, nullptr); }
void destroy(VkImageView image_view) { vkDestroyImageView(vk_context::get().device, image_view, nullptr); }
void destroy(VkSurfaceKHR surface) { SDL_Vulkan_DestroySurface(vk_context::get().instance, surface, nullptr); }
void destroy(VkSwapchainKHR swapchain) { vkDestroySwapchainKHR(vk_context::get().device, swapchain, nullptr); }
void destroy(VkFence fence) { vkDestroyFence(vk_context::get().device, fence, nullptr); }
void destroy(VkSemaphore semaphore) { vkDestroySemaphore(vk_context::get().device, semaphore, nullptr); }
void destroy(SDL_Window* window) { SDL_DestroyWindow(window); }
void destroy(VkSampler sampler) { vkDestroySampler(vk_context::get().device, sampler, nullptr); }
void destroy(VkCommandPool pool) { vkDestroyCommandPool(vk_context::get().device, pool, nullptr); }
void destroy(VkBuffer buffer, VmaAllocation_T* allocation) { vmaDestroyBuffer(vk_context::get().allocator, buffer, allocation); }
void destroy(VkImage image, VmaAllocation_T* allocation) { vmaDestroyImage(vk_context::get().allocator, image, allocation); }
void destroy(VkShaderModule module) { vkDestroyShaderModule(vk_context::get().device, module, nullptr); }
void destroy(VkPipelineLayout layout) { vkDestroyPipelineLayout(vk_context::get().device, layout, nullptr); }
void destroy(VkPipeline pipeline) { vkDestroyPipeline(vk_context::get().device, pipeline, nullptr); }
}