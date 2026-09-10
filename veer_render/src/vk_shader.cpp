#include "vk_shader.h"

#include "shader_utils.h"
#include "vk_context.h"

#include <veer_core/log.h>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

// TODO: actually add destructors, handle nullptrs correctly
namespace ve
{
error_code vk_shader_module::init(const std::filesystem::path& path)
{
    SAFE_HANDLE_EXPECTED(code, utils::load_from_source(path));

    VkShaderModuleCreateInfo shader_module_create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<uint32_t>(code.size),
        .pCode = reinterpret_cast<const uint32_t*>(code.data)
    };
    if (FAILED(vkCreateShaderModule(context.device.vk, &shader_module_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create vulkan shader module");
    
    return error_code::success;
}

void vk_shader_module::destroy()
{
    vkDestroyShaderModule(context.device.vk, vk, nullptr);
    // ugly workaround for deleting type-erased-wise, anyway shaders will be stored differently
    Slang::ComPtr<const ISlangBlob> dummy(reinterpret_cast<const ISlangBlob*>(code.data));
}
}