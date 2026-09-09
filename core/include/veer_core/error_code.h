#pragma once

#include <type_traits>

namespace ve
{
enum class error_code
{
    success = 0,
    initialization,
    already_initialized,
    allocation,
    file_read,
    file_not_exists,
    wrong_file_type,
    window,
    wrong_type,
    unknown,
};
}

#define VEER_FAILED(call) call != 0

#define VEER_SAFE_CALL(call)            \
{                                       \
    ve::error_code res = call;          \
    if (res != ve::error_code::success) \
        return res;                     \
}

#define VEER_SAFE_CALL_EXPECTED(call)   \
{                                       \
    ve::error_code res = call;          \
    if (res != ve::error_code::success) \
        return std::unexpected(res);    \
}

#define VEER_SAFE_JUST_INIT(var, ...) VEER_SAFE_CALL(var.init(__VA_ARGS__))
#define VEER_SAFE_INIT(var, ...) var; VEER_SAFE_JUST_INIT(var, __VA_ARGS__)

#ifndef VEER_KEEP_PREFIX
#define FAILED              VEER_FAILED
#define SAFE_CALL           VEER_SAFE_CALL
#define SAFE_CALL_EXPECTED  VEER_SAFE_CALL_EXPECTED
#define SAFE_JUST_INIT      VEER_SAFE_JUST_INIT
#define SAFE_INIT           VEER_SAFE_INIT
#endif
