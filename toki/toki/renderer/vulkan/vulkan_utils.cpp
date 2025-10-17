#include "toki/renderer/private/vulkan/vulkan_utils.h"

#include <shaderc/shaderc.h>
#include <toki/core/attributes.h>

namespace toki {

VkFormat map_color_format(ColorFormat format) {
	switch (format) {
		case ColorFormat::R8:
			return VK_FORMAT_R8_UNORM;
		case ColorFormat::RGBA8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		default:
			TK_UNREACHABLE();
	}
}

toki::Expected<TempDynamicArray<toki::byte>, RendererErrors> compile_shader(ShaderStageFlags stage, StringView source) {
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	TK_ASSERT(compiler != nullptr);

	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	TK_ASSERT(options != nullptr);

	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

#if defined(TK_DIST)
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
#else
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
	shaderc_compile_options_set_generate_debug_info(options);
#endif

	shaderc_shader_kind shader_kind{};
	switch (stage) {
		case ShaderStageFlags::SHADER_STAGE_VERTEX:
			shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
			break;
		case ShaderStageFlags::SHADER_STAGE_FRAGMENT:
			shader_kind = shaderc_shader_kind::shaderc_fragment_shader;
			break;
		default:
			TK_UNREACHABLE();
	}

	shaderc_compilation_result_t result =
		shaderc_compile_into_spv(compiler, source.data(), source.size(), shader_kind, "", "main", options);

	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
		toki::println("Shader compilation failed: {}\n", shaderc_result_get_error_message(result));
		shaderc_result_release(result);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
		return RendererErrors::ShaderCompileError;
	}

	const size_t length = shaderc_result_get_length(result);
	const char* bytes = shaderc_result_get_bytes(result);

	TempDynamicArray<toki::byte> compiled_data(length);
	toki::memcpy(compiled_data.data(), bytes, length);

	shaderc_result_release(result);
	shaderc_compile_options_release(options);
	shaderc_compiler_release(compiler);

	return compiled_data;
}

VkShaderModule create_shader_module(const VulkanState& state, Span<toki::byte> spirv) {
	VkShaderModuleCreateInfo shader_module_create_info{};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = spirv.size();
	shader_module_create_info.pCode = reinterpret_cast<const u32*>(spirv.data());

	VkShaderModule shader_module;
	VkResult result =
		vkCreateShaderModule(state.logical_device, &shader_module_create_info, state.allocation_callbacks, &shader_module);
	TK_ASSERT(result == VK_SUCCESS);

	return shader_module;
}

u32 find_memory_properties(u32 type_filter, VkMemoryPropertyFlags properties) {
	for (u32 i = 0; i < properties; i++) {
		if (type_filter & (1 << i)) {
			return i;
		}
	}

	TK_ASSERT(false, "Memory type not found on VkMemoryPropertyFlags");
	TK_UNREACHABLE();
}

}  // namespace toki
