#include "graphics.h"

#include "internal/asset_manager.h"
#include "shader.h"
#include "vk_context.h"

namespace ve::gfx
{
error_code init()
{
    SAFE_JUST_INIT(vk::context());
    SAFE_JUST_INIT(assets::manager());
    return error_code::success;
}

void destroy()
{
    vk::context().device.wait_idle();
    shader::destroy_all();
    assets::unload_all();
    vk::context().destroy();
}
}