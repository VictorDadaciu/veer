#pragma once

#include <type_traits>

namespace ve
{
enum class error_code
{
    success = 0,

    allocation,
    command_record,
    file_read,
    file_not_exists,
    initialization,
    not_a_file,
    not_implemented,
    render_submit,
    resource_busy,
    shader,
    synchronization,
    texture,
    vulkan,
    window,
    wrong_file_type,
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

#define VEER_SAFE_CALL_EXPECTED(ret, call) ret; \
{                                               \
    auto res = call;                            \
    if (!res.has_value())                       \
        return std::unexpected(res.error());    \
    ret = *res;                                 \
}

#define VEER_SAFE_CALL_RETURN_EXPECTED(call)    \
{                                               \
    ve::error_code res = call;                  \
    if (res != ve::error_code::success)         \
        return std::unexpected(res);            \
}

#define VEER_SAFE_CALL_HANDLE_EXPECTED(ret, call)   \
{                                                   \
    auto res = call;                                \
    if (!res.has_value())                           \
        return res.error();                         \
    ret = *res;                                     \
}

#define VEER_SAFE_CALL_HANDLE_EXPECTED_MOVE(ret, call)  \
{                                                       \
    auto res = call;                                    \
    if (!res.has_value())                               \
        return res.error();                             \
    ret = std::move(*res);                              \
}

#define VEER_SAFE_CALL_HANDLE_EXPECTED_NEW(ret, call) ret; VEER_SAFE_CALL_HANDLE_EXPECTED(ret, call);
#define VEER_SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE(ret, call) ret; VEER_SAFE_CALL_HANDLE_EXPECTED_MOVE(ret, call);
#define VEER_SAFE_JUST_INIT(var, ...) VEER_SAFE_CALL(var.init(__VA_ARGS__))
#define VEER_SAFE_INIT(var, ...) var; VEER_SAFE_JUST_INIT(var, __VA_ARGS__)

#ifndef VEER_KEEP_PREFIX
#define FAILED                              VEER_FAILED
#define SAFE_CALL                           VEER_SAFE_CALL
#define SAFE_CALL_EXPECTED                  VEER_SAFE_CALL_EXPECTED
#define SAFE_CALL_RETURN_EXPECTED           VEER_SAFE_CALL_RETURN_EXPECTED
#define SAFE_CALL_HANDLE_EXPECTED           VEER_SAFE_CALL_HANDLE_EXPECTED
#define SAFE_CALL_HANDLE_EXPECTED_MOVE      VEER_SAFE_CALL_HANDLE_EXPECTED_MOVE
#define SAFE_CALL_HANDLE_EXPECTED_NEW       VEER_SAFE_CALL_HANDLE_EXPECTED_NEW
#define SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE  VEER_SAFE_CALL_HANDLE_EXPECTED_NEW_MOVE
#define SAFE_INIT                           VEER_SAFE_INIT
#define SAFE_JUST_INIT                      VEER_SAFE_JUST_INIT
#endif
