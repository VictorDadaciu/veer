#pragma once

#include "timing.h"

#include <glm/glm.hpp>

namespace ve
{
enum class mouse_button
{
    left,
    right,
    middle,
    unknown
};

enum class keycode
{
    a = 'a',
    b = 'b',
    c = 'c',
    d = 'd',
    e = 'e',
    f = 'f',
    g = 'g',
    h = 'h',
    i = 'i',
    j = 'j',
    k = 'k',
    l = 'l',
    m = 'm',
    n = 'n',
    o = 'o',
    p = 'p',
    q = 'q',
    r = 'r',
    s = 's',
    t = 't',
    u = 'u',
    v = 'v',
    w = 'w',
    x = 'x',
    y = 'y',
    z = 'z',
    num_1 = '1',
    num_2 = '2',
    num_3 = '3',
    num_4 = '4',
    num_5 = '5',
    num_6 = '6',
    num_7 = '7',
    num_8 = '8',
    num_9 = '9',
    num_0 = '0',
    backtick = '`',
    grave = backtick,
    tab = '\t',
    equals = '=',
    dash = '-',
    minus = dash,
    space = ' ',
    slash = '/',
    dot = '.',
    period = dot,
    comma = ',',
    backslash = '\\',
    apostrophe = '\'',
    semicolon = ';',
    closed_square = ']',
    open_square = '[',
    escape,
    caps_lock,
    left_shift,
    left_ctrl,
    left_alt,
    left_cmd = left_alt,
    right_alt,
    right_cmd = right_alt,
    right_ctrl,
    right_shift,
    enter,
    backspace,
    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,
    insert,
    home,
    page_up,
    page_down,
    del,
    end,
    arrow_up,
    arrow_right,
    arrow_down,
    arrow_left,
    unknown
};
}

namespace ve::inputs
{
void process();

[[nodiscard]] bool quit() noexcept;

// key events
[[nodiscard]] bool just_pressed(ve::keycode) noexcept;
[[nodiscard]] bool just_double_pressed(ve::keycode) noexcept;
[[nodiscard]] bool is_pressed(ve::keycode) noexcept;
[[nodiscard]] bool just_released(ve::keycode) noexcept;
[[nodiscard]] ve::time_point last_pressed(ve::keycode) noexcept;
[[nodiscard]] ve::time_point last_released(ve::keycode) noexcept;

[[nodiscard]] float hold_duration(ve::keycode) noexcept;
void set_hold_duration(ve::keycode, float) noexcept;
[[nodiscard]] float double_press_duration(ve::keycode) noexcept;
void set_double_press_duration(ve::keycode, float) noexcept;

// mouse button events
[[nodiscard]] bool just_pressed(ve::mouse_button) noexcept;
[[nodiscard]] bool just_double_pressed(ve::mouse_button) noexcept;
[[nodiscard]] bool is_pressed(ve::mouse_button) noexcept;
[[nodiscard]] bool just_released(ve::mouse_button) noexcept;
[[nodiscard]] ve::time_point last_pressed(ve::mouse_button) noexcept;
[[nodiscard]] ve::time_point last_released(ve::mouse_button) noexcept;

[[nodiscard]] float hold_duration(ve::mouse_button) noexcept;
void set_hold_duration(ve::mouse_button, float) noexcept;
[[nodiscard]] float double_press_duration(ve::mouse_button) noexcept;
void set_double_press_duration(ve::mouse_button, float) noexcept;

// common button events
[[nodiscard]] inline bool is_holding(auto button) noexcept
{
    if (!is_pressed(button))
        return false;
    return time::duration(last_pressed(button), time::now()) >= hold_duration(button);
}

// mouse positions
[[nodiscard]] const glm::vec2& mouse_rel() noexcept;
[[nodiscard]] const glm::vec2& mouse_abs() noexcept;
[[nodiscard]] bool mouse_moved() noexcept;
}