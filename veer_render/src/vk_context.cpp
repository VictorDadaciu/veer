#include "vk_context.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <vector>

namespace
{
using namespace ve;
const std::vector<c_string> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constinit bool enable_validation_layers = false;
#else
constinit bool enable_validation_layers = true;
#endif

VkDebugUtilsMessengerEXT debug_messenger;

bool check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (c_string layer_name : validation_layers)
    {
        bool layer_found{};
        for (const auto& layer_props : available_layers)
        {
            if (strcmp(layer_name, layer_props.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
        {
            warn("Validation layer support not found");
            return enable_validation_layers = false;
        }
    }

    return true;
}

std::vector<c_string> get_required_extensions()
{
    uint32_t sdl_extension_count = 0;
    const char* const* sdl_extensions{SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count)};
    std::vector<c_string> extensions(sdl_extensions, sdl_extensions + sdl_extension_count);
    if (enable_validation_layers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

c_string severity_as_string(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        return "ERROR";
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        return "WARNING";
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        return "INFO";
    return "VERBOSE";
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*)
{
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && msg_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        debug("VULKAN VALIDATION {}: \n\n{}\n", severity_as_string(msg_severity), callback_data->pMessage);
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info() noexcept
{
    return {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback
    };
}

VkResult create_debug_utils_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info{debug_utils_messenger_create_info()};
    vk_weak_ptr<VkInstance> instance = vk_context::get().instance;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) return func(instance, &create_info, nullptr, &debug_messenger);
    else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_utils_messenger()
{
    vk_weak_ptr<VkInstance> instance = vk_context::get().instance;
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) func(instance, debug_messenger, nullptr);
}
}

namespace ve
{
error_code vk_instance::init()
{
    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "veer",
        .apiVersion = VK_API_VERSION_1_3
    };

    auto extensions{get_required_extensions()};
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    VkInstanceCreateInfo instance_create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    auto debug_create_info = debug_utils_messenger_create_info();
    if (enable_validation_layers && check_validation_layer_support())
    {
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
        instance_create_info.pNext = &debug_create_info;
    }

    if (FAILED(vkCreateInstance(&instance_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to initialize Vulkan instance");

    if (enable_validation_layers && FAILED(create_debug_utils_messenger()))
    {
        enable_validation_layers = false;
        error("Failed to create validation layer messenger");
    }

    return error_code::success;
}

void vk_instance::destroy()
{
    if (enable_validation_layers)
        destroy_debug_utils_messenger();
    vk_unique_ptr<VkInstance>::destroy();
}

error_code vk_physical_device::init(VkPhysicalDevice physical_device)
{
    vk = physical_device;
    VkPhysicalDeviceProperties2 props{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };
    vkGetPhysicalDeviceProperties2(vk, &props);
    name = props.properties.deviceName;
    return error_code::success;
}

error_code vk_context::select_gpu()
{
    uint32_t device_count{0};
    if (FAILED(vkEnumeratePhysicalDevices(vk_context::get().instance, &device_count, nullptr)))
        return error(error_code::initialization, "Failed to enumerate pyhysical devices");
    std::vector<VkPhysicalDevice> devices(device_count);
    gpus.resize(device_count);
    if (FAILED(vkEnumeratePhysicalDevices(vk_context::get().instance, &device_count, devices.data())))
        return error(error_code::initialization, "Failed to enumerate pyhysical devices");

    for (size_t i = 0; i < device_count; ++i)
        SAFE_JUST_INIT(gpus[i], devices[i]);

    gpu_index = 0;
    trace("Selected device: {}", gpu().name);
    return error_code::success;
}

error_code vk_device::init()
{
    auto& queue = vk_context::get().queue;
    const float queue_priorities{1.0f};
    VkDeviceQueueCreateInfo queue_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue.family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priorities
    };

    const std::vector<c_string> device_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceVulkan13Features vk13_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
    };
    VkPhysicalDeviceVulkan12Features vk12_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vk13_features
    };
    VkPhysicalDeviceVulkan11Features vk11_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vk12_features
    };
    VkPhysicalDeviceFeatures2 vk_features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vk11_features
    };
    const auto& gpu = vk_context::get().gpu();
    vkGetPhysicalDeviceFeatures2(gpu, &vk_features2);

    VkDeviceCreateInfo device_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &vk_features2,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
    };

    if (FAILED(vkCreateDevice(gpu, &device_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create logical device");

    vkGetDeviceQueue(vk, queue.family, 0, queue.write());

    return error_code::success;
}

error_code vk_queue::init()
{
    uint32_t queue_family_count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context::get().gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context::get().gpu(), &queue_family_count, queue_families.data());

    bool found = false;
    for (size_t i = 0; i < queue_families.size(); ++i)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            !FAILED(SDL_Vulkan_GetPresentationSupport(vk_context::get().instance, vk_context::get().gpu(), i)))
        {
            family = i;
            found = true;
            break;
        }
    }

    if (!found)
        return error(error_code::initialization, "Failed to find appropriate queue on physical device");

    return error_code::success;
}

error_code vk_allocator::init()
{
    VmaVulkanFunctions vk_funcs{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage,
    };
    VmaAllocatorCreateInfo allocator_create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT, 
        .physicalDevice = vk_context::get().gpu(),
        .device = vk_context::get().device,
        .pVulkanFunctions = &vk_funcs,
        .instance = vk_context::get().instance
    };

    if (FAILED(vmaCreateAllocator(&allocator_create_info, &vk)))
        return error(error_code::initialization, "Failed to initialize VMA allocator");

    return error_code::success;
}

error_code vk_descriptor_set_layout::init()
{
    VkDescriptorBindingFlags desc_variable_flag{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };
    VkDescriptorSetLayoutBindingFlagsCreateInfo desc_binding_flags{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &desc_variable_flag
    };
    VkDescriptorSetLayoutBinding desc_layout_binding_tex{
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutCreateInfo desc_layout_tex_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &desc_binding_flags,
        .bindingCount = 1,
        .pBindings = &desc_layout_binding_tex
    };
    if (FAILED(vkCreateDescriptorSetLayout(vk_context::get().device, &desc_layout_tex_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create descriptor set layout");

    return error_code::success;
}

error_code vk_descriptor_pool::init(size_t max_sets)
{
    VkDescriptorPoolSize pool_size{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = static_cast<uint32_t>(max_sets) // TODO: is this really correct?
    };
    VkDescriptorPoolCreateInfo desc_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(max_sets), // TODO: is this really correct?
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size
    };
    if (FAILED(vkCreateDescriptorPool(vk_context::get().device, &desc_pool_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create descriptor pool");

    return error_code::success;
}

std::expected<vk_weak_ptr<VkDescriptorSet>, error_code> vk_descriptor_pool::allocate_descriptor_set(vk_weak_ptr<VkDescriptorSetLayout> layout)
{
    uint32_t variable_desc_count = 1;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_desc_count_alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &variable_desc_count
    };
    VkDescriptorSetAllocateInfo tex_desc_set_alloc{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variable_desc_count_alloc_info,
        .descriptorPool = vk_context::get().desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = layout.write()
    };
    vk_weak_ptr<VkDescriptorSet> ret{};
    if (FAILED(vkAllocateDescriptorSets(vk_context::get().device, &tex_desc_set_alloc, ret.write())))
        return std::unexpected(error(error_code::allocation, "Failed to allocate descriptor set"));
    return ret;
}

std::expected<std::vector<vk_weak_ptr<VkDescriptorSet>>, error_code> vk_descriptor_pool::allocate_descriptor_sets(vk_weak_ptr<VkDescriptorSetLayout> layout, size_t n)
{
    std::vector<uint32_t> variable_desc_counts(n, 1);
    VkDescriptorSetVariableDescriptorCountAllocateInfo variable_desc_count_alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = static_cast<uint32_t>(n),
        .pDescriptorCounts = variable_desc_counts.data()
    };
    VkDescriptorSetAllocateInfo tex_desc_set_alloc{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variable_desc_count_alloc_info,
        .descriptorPool = vk_context::get().desc_pool,
        .descriptorSetCount = static_cast<uint32_t>(n),
        .pSetLayouts = layout.write()
    };
    std::vector<vk_weak_ptr<VkDescriptorSet>> ret(n);
    if (FAILED(vkAllocateDescriptorSets(vk_context::get().device, &tex_desc_set_alloc, reinterpret_cast<VkDescriptorSet*>(ret.data()))))
        return std::unexpected(error(error_code::allocation, "Failed to allocate descriptor sets"));
    return ret;
}

error_code vk_frame_context::init()
{
    SAFE_JUST_INIT(pool);
    SAFE_CALL_HANDLE_EXPECTED_MOVE(command_buffer, pool.allocate_cmd_buffer());
    SAFE_JUST_INIT(image_acquired_semaphore);
    SAFE_JUST_INIT(render_start_fence);
    return error_code::success;
}

void vk_frame_context::destroy()
{
    image_acquired_semaphore.destroy();
    render_start_fence.destroy();
    pool.destroy();
}

error_code vk_context::init()
{
    SAFE_JUST_INIT(instance);
    SAFE_CALL(select_gpu());
    SAFE_JUST_INIT(queue);
    SAFE_JUST_INIT(device);
    SAFE_JUST_INIT(allocator);
    SAFE_JUST_INIT(layout);
    SAFE_JUST_INIT(desc_pool);
    for (auto& frame : frames)
        SAFE_JUST_INIT(frame);
    return error_code::success;
}

void vk_context::destroy()
{
    for (auto& frame : frames)
        frame.destroy();
    desc_pool.destroy();
    layout.destroy();
    allocator.destroy();
    device.destroy();
    instance.destroy();
}
}
