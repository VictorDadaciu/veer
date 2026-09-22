#include "vk_pipeline.h"

#include "shader.h"
#include "vk_context.h"

#include <veer_core/log.h>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

// TODO: actually add destructors, handle nullptrs correctly
namespace ve
{
error_code vk_shader_module::init(const byte_span& code)
{
    VkShaderModuleCreateInfo shader_module_create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<uint32_t>(code.size),
        .pCode = reinterpret_cast<const uint32_t*>(code.data)
    };
    if (FAILED(vkCreateShaderModule(vk_context::get().device, &shader_module_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create vulkan shader module");

    return error_code::success;
}

void vk_shader_module::destroy()
{
    vkDestroyShaderModule(vk_context::get().device, vk, nullptr);
}

error_code vk_pipeline::init(const byte_span& code)
{
    vk_shader_module SAFE_INIT(module, code);
    error_code res = init(module);
    if (res != error_code::success)
    {
        module.destroy();
        return res;
    }
    module.destroy();
    return error_code::success;
}

error_code vk_pipeline::init(const vk_shader_module& module)
{
    // VkPushConstantRange push_constant_range{
    //     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    //     .size = sizeof(VkDeviceAddress)
    // };

    // VkPipelineLayoutCreateInfo pipeline_layout_create_info{
    //     .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    //     .setLayoutCount = 1,
    // };
    
    

    return error_code::success;
}

void vk_pipeline::destroy()
{
    vkDestroyPipeline(vk_context::get().device, vk, nullptr);
}
}