#include "inputs.h"

#include "internal/input_manager.h"

#include <veer_core/log.h>

#include <SDL3/SDL.h>

namespace
{
using namespace ve;
using namespace ve::inputs;

void process_button_down_event(auto tag)
{
    auto& input = input_manager::get();
    input.set<is_pressed_p>(tag, true);
    input.set<just_pressed_p>(tag, true);
    auto now = time::now();
    if (time::duration(last_pressed(tag), now) >= double_press_duration(tag))
        input.set<just_double_pressed_p>(tag, true);
    input.set<last_pressed_p>(tag, now);
}

void process_button_up_event(auto tag)
{
    auto& input = input_manager::get();
    input.set<is_pressed_p>(tag, false);
    input.set<just_released_p>(tag, true);
    input.set<last_released_p>(tag, time::now());
}

void process_key_down_event(const SDL_KeyboardEvent& e)
{
    keycode code = SDL_keycode_to_veer_keycode(e.key);
    if (code == keycode::unknown)
        return;
    process_button_down_event(code);
}

void process_key_up_event(const SDL_KeyboardEvent& e)
{
    keycode code = SDL_keycode_to_veer_keycode(e.key);
    if (code == keycode::unknown)
        return;
    process_button_up_event(code);
}

void process_mouse_button_down_event(const SDL_MouseButtonEvent& e)
{
    mouse_button button = SDL_mouse_button_to_veer_mouse_button(e.button);
    if (button == mouse_button::unknown)
        return;
    process_button_down_event(button);
}

void process_mouse_button_up_event(const SDL_MouseButtonEvent& e)
{
    mouse_button button = SDL_mouse_button_to_veer_mouse_button(e.button);
    if (button == mouse_button::unknown)
        return;
    process_button_up_event(button);
}

void process_mouse_motion_event(const SDL_MouseMotionEvent& e)
{
    auto& input = input_manager::get();
    input.mouse_rel = glm::vec2(e.xrel, e.yrel);
    input.mouse_abs = glm::vec2(e.x, e.y);
}

void clear_temp_events()
{
    input_db::iterate<SELECT(just_pressed_p, just_released_p, just_double_pressed_p), FROM()>(
        [](const auto&, auto& jp, auto& jr, auto& jdp)
        {
            jp = jr = jdp = false;
        }
    );
    input_manager::get().mouse_rel = glm::vec2();
}
}

namespace ve::inputs
{
using namespace ve;
void process()
{
    clear_temp_events();    

    auto& input = input_manager::get();
    SDL_Event e; // TODO: something with SDL_keymod
    while (SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_EVENT_QUIT:
                trace("Quit requested");
                input.quit = true;
                break;
            case SDL_EVENT_KEY_UP:
                process_key_up_event(e.key);
                break;
            case SDL_EVENT_KEY_DOWN:
                process_key_down_event(e.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                process_mouse_button_up_event(e.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                process_mouse_button_down_event(e.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                process_mouse_motion_event(e.motion);
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
    return input_manager::get().get<just_pressed_p>(code);
}

bool just_double_pressed(keycode code) noexcept
{
    return input_manager::get().get<just_double_pressed_p>(code);
}

bool is_pressed(keycode code) noexcept
{
    return input_manager::get().get<is_pressed_p>(code);
}

bool just_released(keycode code) noexcept
{
    return input_manager::get().get<just_released_p>(code);
}

time_point last_pressed(keycode code) noexcept
{
    return input_manager::get().get<last_pressed_p>(code);
}

time_point last_released(keycode code) noexcept
{
    return input_manager::get().get<last_released_p>(code);
}

float hold_duration(keycode code) noexcept
{
    return input_manager::get().get<hold_duration_p>(code);
}

void set_hold_duration(keycode code, float duration) noexcept
{
    input_manager::get().set<hold_duration_p>(code, std::max(duration, 0.f));
}

float double_press_duration(keycode code) noexcept
{
    return input_manager::get().get<double_press_duration_p>(code);
}

void set_double_press_duration(keycode code, float duration) noexcept
{
    input_manager::get().set<double_press_duration_p>(code, std::max(duration, 0.f));
}

bool just_pressed(mouse_button button) noexcept
{
    return input_manager::get().get<just_pressed_p>(button);
}

bool just_double_pressed(mouse_button button) noexcept
{
    return input_manager::get().get<just_double_pressed_p>(button);
}

bool is_pressed(mouse_button button) noexcept
{
    return input_manager::get().get<is_pressed_p>(button);
}

bool just_released(mouse_button button) noexcept
{
    return input_manager::get().get<just_released_p>(button);
}

time_point last_pressed(mouse_button button) noexcept
{
    return input_manager::get().get<last_pressed_p>(button);
}

time_point last_released(mouse_button button) noexcept
{
    return input_manager::get().get<last_released_p>(button);
}

float hold_duration(mouse_button button) noexcept
{
    return input_manager::get().get<hold_duration_p>(button);
}

void set_hold_duration(mouse_button button, float duration) noexcept
{
    input_manager::get().set<hold_duration_p>(button, std::max(duration, 0.f));
}

float double_press_duration(mouse_button button) noexcept
{
    return input_manager::get().get<double_press_duration_p>(button);
}

void set_double_press_duration(mouse_button button, float duration) noexcept
{
    input_manager::get().set<double_press_duration_p>(button, std::max(duration, 0.f));
}

const glm::vec2& mouse_rel() noexcept
{
    return input_manager::get().mouse_rel;
}

const glm::vec2& mouse_abs() noexcept
{
    return input_manager::get().mouse_abs;
}

bool mouse_moved() noexcept
{
    return input_manager::get().mouse_rel != glm::vec2(); // TODO: fuzzy equa;s
}
}

namespace ve
{
mouse_button SDL_mouse_button_to_veer_mouse_button(size_t code)
{
    switch (code)
    {
        case SDL_BUTTON_LEFT: return mouse_button::left;
        case SDL_BUTTON_RIGHT: return mouse_button::right;
        case SDL_BUTTON_MIDDLE: return mouse_button::middle;
        default: return mouse_button::unknown;
    }
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