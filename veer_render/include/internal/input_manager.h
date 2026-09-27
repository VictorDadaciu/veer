#pragma once

#include <veer_core/db.h>
#include <veer_core/error_code.h>
#include <veer_core/utils.h>

#include <flat_map>

namespace ve
{
DERIVE_PROP(button_state_p, PROPERTY_ROOT(bool));
DERIVE_PROP(just_pressed_p, button_state_p);
DERIVE_PROP(is_pressed_p, button_state_p);
DERIVE_PROP(just_released_p, button_state_p);

using keys = TABLE(
    COLUMN_IMPL(just_pressed_p),
    COLUMN_IMPL(is_pressed_p),
    COLUMN_IMPL(just_released_p)
);

using input_db = database<keys>;

keycode SDL_keycode_to_veer_keycode(size_t);

struct input_manager
{
    VEER_DECLARE_NO_COPY_NO_MOVE(input_manager);
    ~input_manager() = default;

    bool quit{};
    std::flat_map<keycode, keys::index> keycode_map;

    keys::index get_or_insert_in_map(keycode);

    template<class state_t>
        requires(is_descendent_of(^^state_t, ^^button_state_p))
    void set_state(keycode code, bool new_state)
    {
        input_db::cell<state_t>(keys::index(get_or_insert_in_map(code))) = new_state;
    }

    template<class state_t>
        requires(is_descendent_of(^^state_t, ^^button_state_p))
    bool get_state(keycode code)
    {
        return input_db::cell<state_t>(keys::index(get_or_insert_in_map(code)));
    }

    _VEER_SINGLETON(input_manager);
};
}