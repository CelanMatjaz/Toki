#include "shader_compiler.h"

#include <expected>
#include <fstream>
#include <print>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include "toki/core/assert.h"
#include "toki/resources/directory.h"
#include "toki/resources/resource_utils.h"

namespace Toki {

#define RESOURCE_NAME(str) (uint32_t) str[0] | (uint32_t) str[1] << 8 | (uint32_t) str[2] << 16 | (uint32_t) str[3] << 24;

struct Metadata {
    uint32_t resourceName;
    uint32_t resourceVersion;
};

std::vector<uint32_t> ShaderCompiler::compileShader(std::string name, const ShaderSource& source, bool cacheResult) {
    shaderc::Compiler spirvCompiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shaderKind;

    switch (source.shaderStage) {
        case ShaderStage::SHADER_STAGE_VERTEX:
            shaderKind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::SHADER_STAGE_FRAGMENT:
            shaderKind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    shaderc::SpvCompilationResult spirvModule = spirvCompiler.CompileGlslToSpv(source.source.c_str(), shaderKind, "", options);
    if (spirvModule.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        std::println("ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\n", spirvModule.GetErrorMessage(), (int) spirvModule.GetCompilationStatus());
    }
    TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error");

    TK_ASSERT(spirvModule.end() - spirvModule.begin() > 0, "");
    return { spirvModule.begin(), spirvModule.end() };
}

void ShaderCompiler::serializeSPIRV(std::string name, const std::vector<uint32_t>& spirv) {
    std::filesystem::path savePath = Directory::cacheDirectory() / "shaders" / name;
    savePath.replace_extension("shdr");

    ResourceUtils::ensureDirectory(Directory::cacheDirectory() / "shaders");

    std::ofstream file(savePath, std::ios::binary);

    Metadata metadata{};
    metadata.resourceName = RESOURCE_NAME("SHDR");
    metadata.resourceVersion = RESOURCE_NAME("0001");

    uint32_t spirvSize = spirv.size() * sizeof(uint32_t);

    file.write((char*) &metadata, sizeof(Metadata));
    file.write((char*) &spirvSize, sizeof(uint32_t));
    file.write((char*) spirv.data(), spirv.size() * sizeof(uint32_t));
}

std::expected<std::vector<uint32_t>, Error> ShaderCompiler::deserializeSPIRV(std::string name) {
    std::filesystem::path loadPath = Directory::cacheDirectory() / "shaders" / name;

    if (!ResourceUtils::fileExists(loadPath)) {
        return std::unexpected{ Error::FileNotFoundError };
    }

    std::ifstream file(loadPath, std::ios::binary);

    Metadata metadata{};
    file.read((char*) &metadata, sizeof(Metadata));

    uint32_t spirvSize;
    file.read((char*) &spirvSize, sizeof(uint32_t));

    std::vector<uint32_t> spirv(spirvSize / sizeof(uint32_t));
    file.read((char*) spirv.data(), spirvSize);

    return spirv;
}

}  // namespace Toki
