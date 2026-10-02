#include "graphics.h"

#include "staging_buffer.h"
#include "shader.h"
#include "vk_context.h"
#include "vk_utils.h"
#include "window.h"

#include "internal/asset_manager.h"
#include "internal/game_clock.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

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
    vk_frame_context& frame = vk_context::get().current_frame();

    frame.render_start_fence.wait();
    frame.render_start_fence.reset();

    auto& link = win.swapchain.acquire_next(frame.image_acquired_semaphore);
    auto& cb = frame.command_buffer;
    SAFE_CALL_RETURN_EXPECTED(cb.reset());
    SAFE_CALL_RETURN_EXPECTED(cb.begin());
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
                .color{0.02f, 0.02f, 0.02f, 1.f}
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
        auto extent = vk::to_extent_2d(win.size());
        VkRenderingInfo rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{
                .extent = extent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = &depth_attachment_info
        };
        cb.begin_render(rendering_info);

        VkViewport vp{
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };
        VkRect2D scissor{
            .extent = extent
        };
        cb.set_viewport_and_scissor(vp, scissor);
    }
    return cb;
}

error_code end_draw(window& win, vk_command_buffer& cb)
{
    cb.end_render();

    const auto& link = win.swapchain.current_link();
    VkImageMemoryBarrier2 barrier_present{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = link.image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VkDependencyInfo barrier_present_dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier_present
    };
    cb.pipeline_barrier(barrier_present_dependency_info);

    SAFE_CALL(cb.end());

    const auto& ctx = vk_context::get();
    const auto& frame = ctx.current_frame();
    VkSemaphoreSubmitInfo wait_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.image_acquired_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSemaphoreSubmitInfo signal_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = link.render_complete_semaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkCommandBufferSubmitInfo command_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cb
    };
    VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_info,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_info,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_info,
    };
    SAFE_CALL(cb.submit(ctx.queue, submit_info, frame.render_start_fence));

    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = link.render_complete_semaphore.read(),
        .swapchainCount = 1,
        .pSwapchains = win.swapchain.read(),
        .pImageIndices = &win.swapchain.current_image_index
    };
    if (LEGACY_FAILED(vkQueuePresentKHR(ctx.queue, &present_info)))
        return error(error_code::render_submit, "Failed to present swapchain");
    return error_code::success;
}

void advance_frame() noexcept
{
    auto& ctx = vk_context::get();
    ctx.current_frame_index = (ctx.current_frame_index + 1) % vk::frames_in_flight;
    auto& gc = game_clock::get();
    gc.advance_frame();
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