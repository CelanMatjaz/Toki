#include "shader_compiler.h"

#include <shaderc/shaderc.hpp>

namespace toki {

BumpRef<u32> compile_shader(const CompileShaderConfig& config) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_DIST
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shader_kind;

    switch (config.stage) {
        case ShaderStage::VERTEX:
            shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::FRAGMENT:
            shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            TK_ASSERT(false, "Shader stage not supported");
            TK_UNREACHABLE();
    }

    Stream stream(config.source_path, FILE_OPEN_READ | FILE_OPEN_APPEND);
    u32 source_size = stream.tell();
    stream.seek(0);

    DynamicArray<char, BumpAllocator> source_data(config.allocator, source_size);
    stream.read(source_size, source_data.data());

    shaderc::SpvCompilationResult spirv_module =
        compiler.CompileGlslToSpv(source_data.data(), source_size, shader_kind, config.source_path, "main", options);
    if (spirv_module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR(
            "ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}",
            spirv_module.GetErrorMessage(),
            (int) spirv_module.GetCompilationStatus());
    }
    TK_ASSERT(
        spirv_module.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success,
        "Shader compilation error");

    u32 spirv_size = (*spirv_module.end() - *spirv_module.begin());

    return { config.allocator, static_cast<u32>(spirv_size / sizeof(u32)), spirv_module.begin() };

    // VkShaderModuleCreateInfo shader_module_create_info{};
    // shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    // shader_module_create_info.pCode = reinterpret_cast<const u32*>(spirv_module.begin());
    // shader_module_create_info.codeSize = ((uintptr_t) spirv_module.end()) - ((uintptr_t) spirv_module.begin());
    //
    // VkShaderModule shader_module{};
    // VK_CHECK(
    //     vkCreateShaderModule(
    //         mContext->device, &shader_module_create_info, mContext->allocation_callbacks, &shader_module),
    //     "Could not create shader module");
}

}  // namespace toki
