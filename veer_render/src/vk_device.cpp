#include "vk_device.h"

#include "vk_context.h"

#include <veer_core/log.h>
#include <veer_core/utils.h>

#include <source_location>
#include <vector>

namespace ve
{
error_code vk_physical_device::init()
{
    uint32_t device_count{0};
    if (FAILED(vkEnumeratePhysicalDevices(vk::context().instance.vk, &device_count, nullptr)))
        return error(error_code::initialization, "Failed to enumerate pyhysical devices");
    std::vector<VkPhysicalDevice> devices(device_count);
    if (FAILED(vkEnumeratePhysicalDevices(vk::context().instance.vk, &device_count, devices.data())))
        return error(error_code::initialization, "Failed to enumerate pyhysical devices");

    vk = devices[0];

    VkPhysicalDeviceProperties2 device_properties{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    };
    // TODO: not the right way to select physical device
    vkGetPhysicalDeviceProperties2(vk, &device_properties);

    trace("Selected device: {}", device_properties.properties.deviceName);

    return error_code::success;
}

error_code vk_device::init()
{
    SAFE_JUST_INIT(queue);

    const float queue_priorities{1.0f};
    VkDeviceQueueCreateInfo queue_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue.family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priorities
    };

    const std::vector<c_string> device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDeviceVulkan12Features vk12_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features vk13_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vk12_features,
        .synchronization2 = true,
        .dynamicRendering = true
    };
    VkPhysicalDeviceFeatures vk_features{
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo device_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &vk13_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = &vk_features
    };

    if (FAILED(vkCreateDevice(vk::context().physical_device.vk, &device_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create logical device");

    vkGetDeviceQueue(vk, queue.family, 0, &queue.vk);

    return error_code::success;
}

void vk_device::wait_idle() noexcept
{
    vkDeviceWaitIdle(vk);
}

void vk_device::destroy()
{
    vkDestroyDevice(vk, nullptr);
}
}
