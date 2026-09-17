#include "utils.h"

namespace ve
{
std::byte* allocate_cache_aligned_bytes(size_t byte_size)
{
    return reinterpret_cast<std::byte*>(new cache_aligned_bytes[byte_size / cache_line_size]);
}
}