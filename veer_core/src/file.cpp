#include "file.h"

#include "log.h"

#include <filesystem>
#include <fstream>

namespace
{
using namespace ve;
using namespace ve::file;
error_code exists_boilerplate(const std::string& path) noexcept
{
    if (!exists(path))
        return warn(error_code::file_not_exists, "File \"{}\" does not exist", path);
    return error_code::success;
}

error_code check_file_boilerplate(const std::string& path) noexcept
{
    SAFE_CALL(exists_boilerplate(path));
    if (!is_file(path))
        return warn(error_code::not_a_file, "Path \"{}\" is not a file", path);
    return error_code::success;
}
}

namespace ve::file
{
bool exists(const std::string& path) noexcept
{
    return std::filesystem::exists(path);
}

bool is_file(const std::string& path) noexcept
{
    return std::filesystem::is_regular_file(path);
}

bool is_dir(const std::string& path) noexcept
{
    return std::filesystem::is_directory(path);
}

bool is_absolute(const std::string& path) noexcept
{
    return std::filesystem::path(path).is_absolute();
}

bool is_relative(const std::string& path) noexcept
{
    return std::filesystem::path(path).is_relative();
}

std::string absolute(const std::string& path) noexcept
{
    return std::filesystem::absolute(path);
}

std::string stem(const std::string& path) noexcept
{
    return std::filesystem::path(path).stem();
}

std::string filename(const std::string& path) noexcept
{
    return std::filesystem::path(path).filename();
}

std::string extension(const std::string& path) noexcept
{
    return std::filesystem::path(path).extension();
}

std::expected<std::string, error_code> read_entire_file(const std::string& path) noexcept
{
    SAFE_CALL_RETURN_EXPECTED(check_file_boilerplate(path));
    std::ifstream f(path, std::ios::ate | std::ios::binary);
    if (!f.is_open())
    return std::unexpected(error(error_code::file_read, "Failed to read file \"{}\"", path));
    
    std::string ret(f.tellg(), '\0');
    f.seekg(0);
    f.read(&ret[0], ret.size());
    f.close();
    return ret;
}
}