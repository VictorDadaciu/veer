#include "inputs.h"

#include "internal/input_manager.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>

namespace ve::inputs
{
using namespace ve;
void process()
{
    SDL_Delay(500);
    trace("Processing events");
    auto& input = input_manager::get();
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_EVENT_QUIT:
                trace("Quit requested");
                input.quit = true;
                break;
            default:
                break;
        }
    }
}

bool quit_requested() noexcept
{
    return input_manager::get().quit;
}
}