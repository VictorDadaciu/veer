#include "graphics.h"

#include "asset.h"
#include "shader.h"
#include "vk_context.h"

namespace ve::gfx
{
error_code init()
{
    return context.init();
}

void destroy()
{
    vkDeviceWaitIdle(context.device.vk);
    shader::destroy_all();
    assets::unload_all();
    context.destroy();
}
}