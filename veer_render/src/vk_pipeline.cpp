#include "vk_pipeline.h"

#include "shader.h"
#include "vk_context.h"

#include <veer_core/log.h>

#include <glm/glm.hpp>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

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

error_code vk_pipeline::init(const byte_span& code)
{
    vk_shader_module SAFE_INIT(module, code);
    return init(module);
}

error_code vk_pipeline::init(const vk_shader_module& module)
{
    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(VkDeviceAddress)
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = vk_context::get().layout.write(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range
    };
    
    if (FAILED(vkCreatePipelineLayout(vk_context::get().device, &pipeline_layout_create_info, nullptr, layout.write())))
        return error(error_code::initialization, "Failed to initialize pipeline layout");

    const VkVertexInputBindingDescription bindings[] = {
        {
            .binding = 0, // POSITION
            .stride = sizeof(glm::vec3),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        },
        {
            .binding = 1, // NORMAL
            .stride = sizeof(glm::vec3),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        },
        {
            .binding = 2, // UV
            .stride = sizeof(glm::vec2),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    const VkVertexInputAttributeDescription attributes[] = {
        {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0
        },
        {
            .location = 1,
            .binding = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0
        },
        {
            .location = 2,
            .binding = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = 0
        }
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 3,
        .pVertexBindingDescriptions = bindings,
        .vertexAttributeDescriptionCount = 3,
        .pVertexAttributeDescriptions = attributes
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    const VkPipelineShaderStageCreateInfo shader_stages[]{
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = module,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = module,
            .pName = "main"
        }
    };

    VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };
    const VkDynamicState dynamic_states[] { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL
    };

    VkFormat color_format = VK_FORMAT_B8G8R8A8_SRGB;
    VkPipelineRenderingCreateInfo rendering_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &color_format, // swapchain image format
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT // swapchain depth format
    };

    VkPipelineColorBlendAttachmentState blend_attachment{
        .colorWriteMask = 0xF
    };
    VkPipelineColorBlendStateCreateInfo color_blend_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blend_attachment
    };
    VkPipelineRasterizationStateCreateInfo rasterization_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .lineWidth = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisample_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };
    VkGraphicsPipelineCreateInfo pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_create_info,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = &depth_stencil_state,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = layout
    };
    if (FAILED(vkCreateGraphicsPipelines(vk_context::get().device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &vk)))
        return error(error_code::initialization, "Failed to create graphics pipeline");
    return error_code::success;
}
}