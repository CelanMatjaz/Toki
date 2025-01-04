#include "shader_compiler.h"

#include <shaderc/shaderc.hpp>

namespace toki {

std::vector<u32> compile_shader(ShaderStage stage, std::string& source) {
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

    switch (stage) {
        case ShaderStage::Vertex:
            shaderKind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::Fragment:
            shaderKind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    shaderc::SpvCompilationResult spirvModule =
        spirvCompiler.CompileGlslToSpv(source.data(), shaderKind, ".", options);
    if (spirvModule.GetCompilationStatus() !=
        shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR(
            "ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\n",
            spirvModule.GetErrorMessage(),
            (int) spirvModule.GetCompilationStatus());
    }
    TK_ASSERT(
        spirvModule.GetCompilationStatus() ==
            shaderc_compilation_status::shaderc_compilation_status_success,
        "Shader compilation error");

    TK_ASSERT(spirvModule.end() - spirvModule.begin() > 0, "");
    return std::vector<u32>{ spirvModule.begin(), spirvModule.end() };
}

}  // namespace toki
