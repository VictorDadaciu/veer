#include "graphics.h"

#include "staging_buffer.h"
#include "shader.h"
#include "vk_context.h"

#include "internal/asset_manager.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>

namespace ve::gfx
{
error_code init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return error(error_code::initialization, "Failed to initialize SDL3");

    SAFE_JUST_INIT(vk_context::get());
    SAFE_JUST_INIT(asset_manager::get());
    SAFE_JUST_INIT(staging_buffer::get(), 1024);
    return error_code::success;
}

std::expected<vk_command_buffer, error_code> begin_draw(window& win)
{
    vk_context& ctx = vk_context::get();
    vk_frame_context& frame = ctx.current_frame();

    frame.render_start_fence.wait(1000);
    frame.render_start_fence.reset();

    auto& link = win.swapchain.acquire_next(frame.image_acquired_semaphore);
    auto& cb = frame.command_buffer;
    SAFE_CALL_RETURN_EXPECTED(cb.reset());
    SAFE_CALL_RETURN_EXPECTED(cb.begin());
    // TODO: this could all go in window
    {
        const VkImageMemoryBarrier2 barriers[]{
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = link.image,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            },
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = win.swapchain.depth_image,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            }
        };
        VkDependencyInfo dependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = barriers
        };
        cb.pipeline_barrier(dependency);
    }
    {
        VkRenderingAttachmentInfo color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = link.image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{
                .color{0.12f, 0.12f, 0.12f, 1.f}
            }
        };
        VkRenderingAttachmentInfo depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = win.swapchain.depth_image.view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{
                .depthStencil = {1.f, 0}
            }
        };
        VkRenderingInfo rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{
                .extent{
                    .width = static_cast<uint32_t>(win.width()),
                    .height = static_cast<uint32_t>(win.height()),
                }
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
        };
        cb.begin_render(rendering_info);

        VkViewport vp{
            .width = static_cast<float>(win.width()),
            .height = static_cast<float>(win.height()),
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };
        VkRect2D scissor{
            .extent{
                .width = static_cast<uint32_t>(win.width()),
                .height = static_cast<uint32_t>(win.height()),
            }
        };
        cb.set_viewport_and_scissor(vp, scissor);
    }
    return cb;
}

error_code end_draw(vk_command_buffer& cb)
{
    SAFE_CALL(cb.end());
    return error_code::success;
}

void destroy()
{
    vk_context::get().device.wait_idle();
    trace("Destroying all shader-related resources...");
    shader::destroy_all();
    trace("Destroying staging buffer...");
    staging_buffer::get().destroy();
    trace("Destroying all assets...");
    assets::unload_all();
    trace("Destroying vulkan context...");
    vk_context::get().destroy();
    trace("Quitting SDL3...");
    SDL_Quit();
}
}