#pragma once

#include "error_code.h"

#include <cstdint>
#include <cstring>
#ifndef VEER_DONT_USE_FORMAT
#include <format>
#endif
#include <iostream>
#include <string>

namespace ve::log
{
enum class level : uint8_t
{
    debug   = 0,
    trace   = 1,
    info    = 2,
    warning = 3,
    error   = 4,
    quiet   = 5,
};

void init(
    const std::string& = "veer_log",
    level = level::info,
    std::ostream& = std::cout);

void set_level(level) noexcept;
}

namespace ve
{
void debug(const std::string&);
void trace(const std::string&);
void info(const std::string&);
void warn(const std::string&);
void error(const std::string&);
[[nodiscard]] error_code error(error_code, const std::string&);

#ifndef VEER_DONT_USE_FORMAT
template<typename... args_t>
inline void debug(std::format_string<args_t...> format_string, args_t&&... args)
{
    debug(std::format(format_string, args...));
}

template<typename... args_t>
inline void trace(std::format_string<args_t...> format_string, args_t&&... args)
{
    trace(std::format(format_string, args...));
}

template<typename... args_t>
inline void info(std::format_string<args_t...> format_string, args_t&&... args)
{
    info(std::format(format_string, args...));
}

template<typename... args_t>
inline void warn(std::format_string<args_t...> format_string, args_t&&... args)
{
    warn(std::format(format_string, args...));
}

template<typename... args_t>
inline void error(std::format_string<args_t...> format_string, args_t&&... args)
{
    error(std::format(format_string, args...));
}

template<typename... args_t>
[[nodiscard]] inline error_code error(error_code err, std::format_string<args_t...> format_string, args_t&&... args)
{
    return error(err, std::format(format_string, args...));
}
#endif
}