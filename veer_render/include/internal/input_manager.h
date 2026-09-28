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
DERIVE_PROP(just_double_pressed_p, button_state_p);

DERIVE_PROP(time_p, PROPERTY_ROOT(time_point));
DERIVE_PROP(last_pressed_p, time_p);
DERIVE_PROP(last_released_p, time_p);

DERIVE_PROP(duration_p, PROPERTY_ROOT(float));
DERIVE_PROP(hold_duration_p, duration_p);
DERIVE_PROP(double_press_duration_p, duration_p);

using keys = TABLE(
    COLUMN_IMPL(just_pressed_p),
    COLUMN_IMPL(just_released_p),
    COLUMN_IMPL(just_double_pressed_p),
    COLUMN_IMPL(is_pressed_p),
    COLUMN_IMPL(last_pressed_p),
    COLUMN_IMPL(last_released_p),
    COLUMN_IMPL(hold_duration_p),
    COLUMN_IMPL(double_press_duration_p)
);

using mouse_buttons = TABLE(
    PACKED_COLUMN(
        IMPL(just_pressed_p),
        IMPL(just_released_p),
        IMPL(just_double_pressed_p),
        IMPL(is_pressed_p)
    ),
    COLUMN_IMPL(last_pressed_p),
    COLUMN_IMPL(last_released_p),
    COLUMN_IMPL(hold_duration_p),
    COLUMN_IMPL(double_press_duration_p)
);

using input_db = database<keys, mouse_buttons>;

keycode SDL_keycode_to_veer_keycode(size_t);
mouse_button SDL_mouse_button_to_veer_mouse_button(size_t);

struct input_manager
{
private:
    template<typename button_t, class table_t>
    inline row_index<table_t> get_or_insert_in_map(button_t button, std::flat_map<button_t, row_index<table_t>>& map)
    {
        auto it = map.find(button);
        if (it == map.end())
            return map[button] = input_db::push_back(typename table_t::row{});
        else
            return it->second;
    }

public:
    VEER_DECLARE_NO_COPY_NO_MOVE(input_manager);
    ~input_manager() = default;

    bool quit{};
    std::flat_map<keycode, keys::index> keycode_map;
    std::flat_map<mouse_button, mouse_buttons::index> mouse_button_map;

    glm::vec2 mouse_rel;
    glm::vec2 mouse_abs;

    decltype(auto) get_or_insert_in_map(auto tag)
    {
        if constexpr (std::is_same_v<decltype(tag), keycode>)
        {
            return get_or_insert_in_map(tag, keycode_map);
        }
        else
        {
            return get_or_insert_in_map(tag, mouse_button_map);
        }
    }

    template<class prop_t>
    void set(auto tag, auto value)
    {
        input_db::cell<prop_t>(get_or_insert_in_map(tag)) = value;
    }

    template<class prop_t>
    auto get(auto tag)
    {
        return input_db::cell<prop_t>(get_or_insert_in_map(tag));
    }

    _VEER_SINGLETON(input_manager);
};
}