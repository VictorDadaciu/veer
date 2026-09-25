#pragma once

#include <veer_core/error_code.h>
#include <veer_core/utils.h>

namespace ve
{
struct input_manager
{
    VEER_DECLARE_NO_COPY_NO_MOVE(input_manager);
    ~input_manager() = default;

    bool quit{};

    _VEER_SINGLETON(input_manager);
};
}