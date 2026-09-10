#include "file_utils.h"

#include "log.h"

#include <fstream>

namespace ve::utils
{
std::expected<byte_span, error_code> read_entire_file(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "File \"{}\" does not exist", path.display_string()));
    byte_span ret{};
    std::ifstream f(path, std::ios::ate | std::ios::binary);
    if (!f.is_open())
        return std::unexpected(error(error_code::file_read, "Failed to read file \"{}\"", path.display_string()));

    ret.size = static_cast<size_t>(f.tellg());
    f.seekg(0);
    f.read(const_cast<char*>(reinterpret_cast<const char*>(ret.data)), ret.size);
    f.close();
    return ret;
}
}