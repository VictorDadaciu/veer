#include "shader_utils.h"

#include <veer_core/log.h>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <array>

namespace
{
using namespace ve;
static Slang::ComPtr<slang::IGlobalSession> slang_global_session{};

std::expected<byte_span, error_code> load_slang_shader(const std::filesystem::path& path)
{
    SlangGlobalSessionDesc desc{};
    if (!slang_global_session && FAILED(slang::createGlobalSession(&desc, slang_global_session.writeRef())))
        return std::unexpected(error(error_code::shader, "Failed to initialize slang global session"));

    auto slang_targets{
        std::to_array<slang::TargetDesc>({{
            .format{SLANG_SPIRV},
            .profile{slang_global_session->findProfile("spirv_1_4")}
        }})
    };

    auto slang_options{
        std::to_array<slang::CompilerOptionEntry>({{
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1}
        }})
    };

    slang::SessionDesc slang_session_desc{
        .targets{slang_targets.data()},
        .targetCount{static_cast<SlangInt>(slang_targets.size())},
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries{slang_options.data()},
        .compilerOptionEntryCount{static_cast<uint32_t>(slang_options.size())}
    };
    Slang::ComPtr<slang::ISession> slang_session{};
    if (FAILED(slang_global_session->createSession(slang_session_desc, slang_session.writeRef())))
        return std::unexpected(error(error_code::shader, "Failed to initialize slang session"));
        
    Slang::ComPtr<slang::IModule> slang_module{
        slang_session->loadModuleFromSource("triangle", path.display_string().c_str(), nullptr, nullptr)
    };

    if (!slang_module.readRef())
        return std::unexpected(error(error_code::file_read, "Failed to read shader file \"{}\"", path.display_string()));
        
    ISlangBlob* spirv{};
    if (FAILED(slang_module->getTargetCode(0, &spirv)))
        return std::unexpected(error(error_code::shader, "Failed to compiler shader code for \"{}\"", path.display_string()));

    return byte_span{
        .data = reinterpret_cast<const std::byte*>(spirv->getBufferPointer()),
        .size = spirv->getBufferSize()
    };
}
}

namespace ve::utils
{
std::expected<byte_span, error_code> load_from_source(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "Shader file \"{}\" doesn't exist", path));
    std::string extension = path.extension().display_string();
    info("Loading shader file at \"{}\"...", path);
    if (extension == ".slang")
    {
        return load_slang_shader(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid shader file type \"{}\"", path));
    }
}
}