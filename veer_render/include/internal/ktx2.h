#pragma once

#include "vk_context.h"

#include "internal/asset_manager.h"

struct ktxTexture2;
namespace ve::assets
{
class ktx2_texture_wrapper
{
public:
    DECLARE_NO_COPY_NO_MOVE(ktx2_texture_wrapper);

    ~ktx2_texture_wrapper();

    error_code load(const std::string&);
    error_code initialize_texture(ve::texture&);

private:
    ktxTexture2* m_ktx{};
    // TODO: not here
    buffer m_staging_buffer{};
    // TODO: not with fence, just work with a placeholder until copying tex is done
    VkFence m_one_shot_fence{};
};
}