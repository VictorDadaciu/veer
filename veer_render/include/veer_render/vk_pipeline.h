#pragma once

#include "graphics.h"
#include "uniform_buffer.h"
#include "vk_ptr.h"

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <array>
#include <string>

namespace ve
{
struct vk_shader_module : public vk_unique_ptr<VkShaderModule>
{
    VEER_DECLARE_NO_COPY(vk_shader_module);
    vk_shader_module(vk_shader_module&& other) = default;
    vk_shader_module& operator=(vk_shader_module&&) = default;

    error_code init(const std::string&);
    using vk_unique_ptr<VkShaderModule>::destroy;
};

struct camera_data
{
    glm::mat4 proj{};
    glm::mat4 view{};
};

struct model_data
{
    glm::mat4 transform{};
    uint32_t tex_index{};
};

struct pipeline_frame_data
{
    error_code init();
    void destroy();

    uniform_buffer<camera_data> globals{};
    uniform_buffer<model_data> model{};
};

struct vk_pipeline : vk_unique_ptr<VkPipeline>
{
    VEER_DECLARE_NO_COPY(vk_pipeline);
    vk_pipeline(vk_pipeline&& other) :
        vk_unique_ptr<VkPipeline>(std::move(other)),
        frames(std::move(other.frames)),
        layout(std::move(other.layout)) {}

    vk_pipeline& operator=(vk_pipeline&& other)
    {
        vk_unique_ptr<VkPipeline>::operator=(std::move(other));
        frames = std::move(other.frames);
        layout = std::move(other.layout);
        return *this;
    }

    error_code init(const std::string&);
    error_code init(const vk_shader_module&);
    
    void destroy()
    {
        vk_unique_ptr<VkPipeline>::destroy();
        for (auto& frame : frames)
            frame.destroy();
        layout.destroy();
    }

    pipeline_frame_data& current_frame() noexcept;
    const pipeline_frame_data& current_frame() const noexcept;

    std::array<pipeline_frame_data, vk::frames_in_flight> frames{};
    vk_unique_ptr<VkPipelineLayout> layout{};
};
}