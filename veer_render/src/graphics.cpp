#include "graphics.h"

#include "vk_context.h"

namespace ve::gfx
{
error_code init()
{
    return context.init();
}

void destroy()
{
    context.destroy();
}
}