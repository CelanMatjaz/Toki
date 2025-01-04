#include "vulkan_shader.h"

#include "core/scope_wrapper.h"
#include "renderer/vulkan/macros.h"
#include "renderer/vulkan/utils/shader_compiler.h"
#include "resources/loaders/text.h"
#include "vulkan/vulkan_core.h"

namespace toki {

VulkanShader::VulkanShader(RendererContext ctx, const Config& config): Shader(config) {
    std::string vertex_shader_source = read_text_file(config.vertex_shader_path);
    std::vector vertex_shader_binary = compile_shader(ShaderStage ::Fragment, vertex_shader_source);
    Scope<VkShaderModule, VK_NULL_HANDLE> vertex_shader_module(
        create_shader_module(ctx, vertex_shader_binary), [ctx](VkShaderModule sm) {
            vkDestroyShaderModule(ctx.device, sm, ctx.allocationCallbacks);
        });

    std::string fragment_shader_source = read_text_file(config.fragment_shader_path);
    std::vector fragment_shader_binary =
        compile_shader(ShaderStage ::Fragment, fragment_shader_source);
    Scope<VkShaderModule, VK_NULL_HANDLE> fragment_shader_module(
        create_shader_module(ctx, fragment_shader_binary), [ctx](VkShaderModule sm) {
            vkDestroyShaderModule(ctx.device, sm, ctx.allocationCallbacks);
        });

    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    fragment_shader_stage_create_info.module = fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_create_info,
                                                        fragment_shader_stage_create_info };
}

VkShaderModule VulkanShader::create_shader_module(RendererContext ctx, std::vector<u32>& binary) {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = binary.size();
    shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(binary.data());

    VkShaderModule shader_module{};
    TK_ASSERT_VK_RESULT(
        vkCreateShaderModule(
            ctx.device, &shader_module_create_info, ctx.allocationCallbacks, &shader_module),
        "Could not create shader module");

    return shader_module;
}

}  // namespace toki
