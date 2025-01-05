#include "pipeline_utils.h"

#include <shaderc/shaderc.hpp>

#include "renderer/vulkan/macros.h"

namespace toki {

VkShaderModule create_shader_module(Ref<RendererContext> ctx, std::vector<u32>& binary) {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = binary.size() * 4;
    shader_module_create_info.pCode = binary.data();

    VkShaderModule shader_module{};
    TK_ASSERT_VK_RESULT(
        vkCreateShaderModule(ctx->device, &shader_module_create_info, ctx->allocationCallbacks, &shader_module),
        "Could not create shader module");

    return shader_module;
}

std::vector<u32> compile_shader(ShaderStage stage, std::string& source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shader_kind;

    switch (stage) {
        case ShaderStage::Vertex:
            shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::Fragment:
            shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    shaderc::SpvCompilationResult spirv_module = compiler.CompileGlslToSpv(source.data(), shader_kind, ".", options);
    if (spirv_module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR(
            "ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\nSOURCE: {}",
            spirv_module.GetErrorMessage(),
            (int) spirv_module.GetCompilationStatus(),
            source);
    }
    TK_ASSERT(
        spirv_module.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success,
        "Shader compilation error");

    TK_ASSERT(spirv_module.end() - spirv_module.begin() > 0, "");
    return std::vector<u32>{ spirv_module.begin(), spirv_module.end() };
}

}  // namespace toki
