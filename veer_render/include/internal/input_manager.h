#pragma once

#include "inputs.h"

#include <veer_core/db.h>
#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <flat_map>

namespace ve
{
DERIVE_PROP(button_state_flags_p, PROPERTY_ROOT(size_t));

DERIVE_PROP(time_p, PROPERTY_ROOT(time_point));
DERIVE_PROP(last_pressed_p, time_p);
DERIVE_PROP(last_released_p, time_p);

DERIVE_PROP(duration_p, PROPERTY_ROOT(float));
DERIVE_PROP(hold_duration_p, duration_p);
DERIVE_PROP(double_press_duration_p, duration_p);

enum button_state
{
    e_just_pressed,
    e_just_released,
    e_just_double_pressed,
    e_is_pressed,
};

using buttons = TABLE(
    COLUMN_IMPL_AS(button_state_flags_p, uint8_t),
    COLUMN_IMPL(last_pressed_p),
    COLUMN_IMPL(last_released_p),
    COLUMN_IMPL(hold_duration_p),
    COLUMN_IMPL(double_press_duration_p)
);

using input_db = database<buttons>;

inline table_row<buttons> default_button_row()
{
    table_row<buttons> row{};
    row.cell<hold_duration_p>() = 0.4f;
    row.cell<double_press_duration_p>() = 0.25f;
    return row;
}

keycode SDL_keycode_to_veer_keycode(size_t);
mouse_button SDL_mouse_button_to_veer_mouse_button(size_t);

struct input_manager
{
public:
    VEER_DECLARE_NO_COPY_NO_MOVE(input_manager);
    ~input_manager() = default;

    error_code init();

    bool quit{};
    std::flat_map<keycode, buttons::index> keycode_map;

    glm::vec2 mouse_rel;
    glm::vec2 mouse_abs;

    row_index<buttons> get_or_insert_in_keycode_map(keycode);

    _VEER_SINGLETON(input_manager);
};
}