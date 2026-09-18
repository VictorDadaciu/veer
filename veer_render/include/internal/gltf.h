#pragma once

#include "internal/asset_manager.h"

#define TINYGLTF3_ENABLE_FS
#include <tinygltf/tiny_gltf_v3.h>

#include <flat_map>

namespace ve::assets
{
class gltf_model_wrapper : public tg3_model
{
public:
    VEER_DECLARE_NO_COPY_NO_MOVE(gltf_model_wrapper);

    ~gltf_model_wrapper()
    {
        tg3_model_free(this);
    }

    error_code load(const std::string&);
    error_code initialize_mesh(ve::mesh&, size_t);

private:
    size_t get_buffer_and_update_buffers_if_needed(ve::mesh&, size_t);
    void allocate_and_copy(ve::mesh&);

    std::flat_map<size_t, size_t> m_buffer_indices_map{};
    size_t m_total_size;
};
}