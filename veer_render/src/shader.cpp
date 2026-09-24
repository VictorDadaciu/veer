#include "shader.h"

#include "vk_pipeline.h"

#include <veer_core/file.h>
#include <veer_core/log.h>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include <array>
#include <vector>

namespace
{
using namespace ve;
static Slang::ComPtr<slang::IGlobalSession> slang_global_session{};
static Slang::ComPtr<slang::ISession> slang_session{};

std::vector<vk_pipeline> pipelines{};

error_code init_slang()
{
    SlangGlobalSessionDesc desc{};
    if (FAILED(slang::createGlobalSession(&desc, slang_global_session.writeRef())))
        return error(error_code::shader, "Failed to initialize slang global session");

    auto slang_targets{
        std::to_array<slang::TargetDesc>({{
            .format{SLANG_SPIRV},
            .profile{slang_global_session->findProfile("spirv_1_5")}
        }})
    };

    auto slang_options{
        std::to_array<slang::CompilerOptionEntry>({{
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1}
        }})
    };

    c_string search_paths[] = { "/home/victordadaciu/workspace/veer/tests/assets" };
    slang::SessionDesc slang_session_desc{
        .targets{slang_targets.data()},
        .targetCount{static_cast<SlangInt>(slang_targets.size())},
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .searchPaths = search_paths,
        .searchPathCount = 1,
        .compilerOptionEntries{slang_options.data()},
        .compilerOptionEntryCount{static_cast<uint32_t>(slang_options.size())}
    };
    if (!slang_session && FAILED(slang_global_session->createSession(slang_session_desc, slang_session.writeRef())))
        return error(error_code::shader, "Failed to initialize slang session");

    return error_code::success;
}

std::expected<byte_span, error_code> load_slang_shader(const std::string& path)
{
    if (!slang_global_session) SAFE_CALL_RETURN_EXPECTED(init_slang());
    
    Slang::ComPtr<ISlangBlob> errs{};
    Slang::ComPtr<slang::IModule> slang_module{
        slang_session->loadModuleFromSource(
            file::stem(path).c_str(),
            file::absolute(path).c_str(),
            nullptr,
            errs.writeRef()
        )
    };

    if (!slang_module)
    {
        if (errs) debug(reinterpret_cast<c_string>(errs->getBufferPointer()));
        return std::unexpected(error(error_code::file_read, "Failed to read shader file \"{}\"", path));
    }
        
    Slang::ComPtr<ISlangBlob> spirv{};
    if (FAILED(slang_module->getTargetCode(0, spirv.writeRef())))
        return std::unexpected(error(error_code::shader, "Failed to compiler shader code for \"{}\"", path));

    info("Loaded shader file: \"{}\" under name \"{}\"", path, file::stem(path));
    return byte_span{
        .data = reinterpret_cast<const std::byte*>(spirv->getBufferPointer()),
        .size = spirv->getBufferSize()
    };
}

std::expected<size_t, error_code> create_pipeline(const byte_span& code)
{
    vk_pipeline pipeline{};
    SAFE_CALL_RETURN_EXPECTED(pipeline.init(code));
    size_t ret = pipelines.size();
    pipelines.push_back(std::move(pipeline));
    return ret;
}

std::expected<size_t, error_code> create_pipeline_from_source(const std::string& path)
{
    byte_span SAFE_CALL_EXPECTED(code, load_slang_shader(path));
    return create_pipeline(code);
}

std::expected<size_t, error_code> create_pipeline_from_binary(const std::string& path)
{
    byte_span SAFE_CALL_EXPECTED(code, file::read_entire_file(path));
    return create_pipeline(code);
}
}

namespace ve::shader
{
std::expected<size_t, error_code> load(const std::string& path)
{
    if (!file::exists(path))
        return std::unexpected(error(error_code::file_not_exists, "Shader file \"{}\" doesn't exist", path));
    std::string extension = file::extension(path);
    info("Loading shader file at \"{}\"...", path);
    if (extension == ".slang")
    {
        return create_pipeline_from_source(path);
    }
    else if (extension == ".spv" || extension == ".spirv")
    {
        return create_pipeline_from_binary(path);
    }
    else
    {
        return std::unexpected(error(error_code::wrong_file_type, "Invalid shader file type \"{}\"", path));
    }
}

void destroy_all()
{
    for (auto& pipeline : pipelines)
        pipeline.destroy();
    pipelines.clear();
}
}