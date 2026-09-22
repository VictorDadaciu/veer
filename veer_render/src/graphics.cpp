#include "graphics.h"

#include "internal/asset_manager.h"
#include "internal/staging_buffer.h"
#include "shader.h"
#include "vk_context.h"

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