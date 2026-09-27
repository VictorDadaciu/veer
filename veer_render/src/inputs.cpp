#include "inputs.h"

#include "internal/input_manager.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>

namespace
{
using namespace ve;
using namespace ve::inputs;

void process_key_down_event(const SDL_KeyboardEvent& e)
{
    keycode code = SDL_keycode_to_veer_keycode(e.key);
    if (code == keycode::unknown)
        return;
    input_manager::get().set_state<ve::just_pressed_p>(code, true);
    input_manager::get().set_state<ve::is_pressed_p>(code, true);
}

void process_key_up_event(const SDL_KeyboardEvent& e)
{
    keycode code = SDL_keycode_to_veer_keycode(e.key);
    if (code == keycode::unknown)
        return;
    input_manager::get().set_state<ve::is_pressed_p>(code, false);
    input_manager::get().set_state<ve::just_released_p>(code, true);
}
}

namespace ve::inputs
{
using namespace ve;
void process()
{
    input_db::iterate<SELECT(just_pressed_p, just_released_p), FROM()>(
        [](const auto&, auto& jp, auto& jr)
        {
            jp = jr = false;
        }
    );

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
            case SDL_EVENT_KEY_DOWN:
                process_key_down_event(e.key);
                break;
            case SDL_EVENT_KEY_UP:
                process_key_up_event(e.key);
                break;
            default:
                break;
        }
    }
}

bool quit() noexcept
{
    return input_manager::get().quit;
}

bool just_pressed(keycode code) noexcept
{
    return input_manager::get().get_state<ve::just_pressed_p>(code);
}

bool is_pressed(keycode code) noexcept
{
    return input_manager::get().get_state<ve::is_pressed_p>(code);
}

bool just_released(keycode code) noexcept
{
    return input_manager::get().get_state<ve::just_released_p>(code);
}
}

namespace ve
{
keys::index input_manager::get_or_insert_in_map(keycode code)
{
    auto it = keycode_map.find(code);
    if (it == keycode_map.end())
        return keycode_map[code] = input_db::push_back(keys::row{});
    else
        return it->second;
}

keycode SDL_keycode_to_veer_keycode(size_t code)
{
    switch (code)
    {
        case SDLK_RETURN: return keycode::enter;
        case SDLK_ESCAPE: return keycode::escape;
        case SDLK_BACKSPACE: return keycode::backspace;
        case SDLK_TAB: return keycode::tab;
        case SDLK_SPACE: return keycode::space;
        case SDLK_APOSTROPHE: return keycode::apostrophe;
        case SDLK_COMMA: return keycode::comma;
        case SDLK_MINUS: return keycode::minus;
        case SDLK_PERIOD: return keycode::period;
        case SDLK_SLASH: return keycode::slash;
        case SDLK_0: return keycode::num_0;
        case SDLK_1: return keycode::num_1;
        case SDLK_2: return keycode::num_2;
        case SDLK_3: return keycode::num_3;
        case SDLK_4: return keycode::num_4;
        case SDLK_5: return keycode::num_5;
        case SDLK_6: return keycode::num_6;
        case SDLK_7: return keycode::num_7;
        case SDLK_8: return keycode::num_8;
        case SDLK_9: return keycode::num_9;
        case SDLK_SEMICOLON: return keycode::semicolon;
        case SDLK_EQUALS: return keycode::equals;
        case SDLK_LEFTBRACKET: return keycode::open_square;
        case SDLK_BACKSLASH: return keycode::backslash;
        case SDLK_RIGHTBRACKET: return keycode::closed_square;
        case SDLK_GRAVE: return keycode::grave;
        case SDLK_A: return keycode::a;
        case SDLK_B: return keycode::b;
        case SDLK_C: return keycode::c;
        case SDLK_D: return keycode::d;
        case SDLK_E: return keycode::e;
        case SDLK_F: return keycode::f;
        case SDLK_G: return keycode::g;
        case SDLK_H: return keycode::h;
        case SDLK_I: return keycode::i;
        case SDLK_J: return keycode::j;
        case SDLK_K: return keycode::k;
        case SDLK_L: return keycode::l;
        case SDLK_M: return keycode::m;
        case SDLK_N: return keycode::n;
        case SDLK_O: return keycode::o;
        case SDLK_P: return keycode::p;
        case SDLK_Q: return keycode::q;
        case SDLK_R: return keycode::r;
        case SDLK_S: return keycode::s;
        case SDLK_T: return keycode::t;
        case SDLK_U: return keycode::u;
        case SDLK_V: return keycode::v;
        case SDLK_W: return keycode::w;
        case SDLK_X: return keycode::x;
        case SDLK_Y: return keycode::y;
        case SDLK_Z: return keycode::z;
        case SDLK_DELETE: return keycode::del;
        case SDLK_CAPSLOCK: return keycode::caps_lock;
        case SDLK_F1: return keycode::f1;
        case SDLK_F2: return keycode::f2;
        case SDLK_F3: return keycode::f3;
        case SDLK_F4: return keycode::f4;
        case SDLK_F5: return keycode::f5;
        case SDLK_F6: return keycode::f6;
        case SDLK_F7: return keycode::f7;
        case SDLK_F8: return keycode::f8;
        case SDLK_F9: return keycode::f9;
        case SDLK_F10: return keycode::f10;
        case SDLK_F11: return keycode::f11;
        case SDLK_F12: return keycode::f12;
        case SDLK_INSERT: return keycode::insert;
        case SDLK_HOME: return keycode::home;
        case SDLK_PAGEUP: return keycode::page_up;
        case SDLK_END: return keycode::end;
        case SDLK_PAGEDOWN: return keycode::page_down;
        case SDLK_RIGHT: return keycode::arrow_right;
        case SDLK_LEFT: return keycode::arrow_left;
        case SDLK_DOWN: return keycode::arrow_down;
        case SDLK_UP: return keycode::arrow_up;
        default: return keycode::unknown;
    }
}
}