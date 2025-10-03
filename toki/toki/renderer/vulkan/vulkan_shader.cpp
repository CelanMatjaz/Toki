#include <shaderc/shaderc.h>
#include <toki/core/core.h>
#include <toki/renderer/private/vulkan/vulkan_backend.h>
#include <vulkan/vulkan.h>

namespace toki::renderer {

void VulkanBackend::destroy_handle(ShaderHandle handle) {
	TK_ASSERT(m_resources.shaders.exists(handle));
	vkDestroyPipeline(m_state.device, m_resources.shaders[handle].pipeline, m_state.allocation_callbacks);
	m_resources.shaders.invalidate(handle);
}

void VulkanBackend::destroy_handle(ShaderLayoutHandle handle) {
	TK_ASSERT(m_resources.shader_layouts.exists(handle));
	auto layout = m_resources.shader_layouts[handle].pipeline_layout;
	vkDestroyPipelineLayout(
		m_state.device, m_resources.shader_layouts[handle].pipeline_layout, m_state.allocation_callbacks);
	m_resources.shaders.invalidate(handle);
}

ShaderHandle VulkanBackend::create_shader(const ShaderConfig& config) {
	VulkanShader shader = create_vulkan_shader(config);
	Handle handle = m_resources.shaders.emplace_at_first(shader);
	TK_ASSERT(handle);
	return { handle };
}

ShaderLayoutHandle VulkanBackend::create_shader_layout(const ShaderLayoutConfig& config) {
	VulkanShaderLayout shader_layout = create_vulkan_shader_layout(config);
	Handle handle = m_resources.shader_layouts.emplace_at_first(shader_layout);
	TK_ASSERT(handle);
	return { handle };
}

VulkanShader VulkanBackend::create_vulkan_shader(const ShaderConfig& config) {
	TK_ASSERT(config.color_formats.size() > 0);

	TempDynamicArray<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
	shader_stage_create_infos.reserve(ShaderStage::SHADER_STAGE_SIZE);

	for (u32 i = 0; i < config.sources.size(); i++) {
		if (config.sources[i].size() == 0) {
			continue;
		}

		auto compile_shader_result = compile_shader(static_cast<ShaderStage>(i), config.sources[i]);
		TK_ASSERT(compile_shader_result);

		shader_stage_create_infos.emplace_back({});
		VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_stage_create_infos.last();

		shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_create_info.module = create_shader_module(compile_shader_result.value());
		shader_stage_create_info.pName = "main";

		switch (static_cast<ShaderStage>(i)) {
			case SHADER_STAGE_VERTEX:
				shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case SHADER_STAGE_FRAGMENT:
				shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			default:
				break;
		}
	}

	TempDynamicArray<VkVertexInputBindingDescription> vertex_binding_descriptions(config.bindings.size());
	for (u32 i = 0; i < vertex_binding_descriptions.size(); i++) {
		vertex_binding_descriptions[i].binding = config.bindings[i].binding;
		vertex_binding_descriptions[i].stride = config.bindings[i].stride;

		switch (config.bindings[i].inputRate) {
			case VertexInputRate::VERTEX:
				vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				break;
			case VertexInputRate::INSTANCE:
				vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				break;
		}
	}

	TempDynamicArray<VkVertexInputAttributeDescription> vertex_attribute_descriptions(config.attributes.size());
	for (u32 i = 0; i < vertex_attribute_descriptions.size(); i++) {
		vertex_attribute_descriptions[i].binding = config.attributes[i].binding;
		vertex_attribute_descriptions[i].offset = config.attributes[i].offset;
		vertex_attribute_descriptions[i].location = config.attributes[i].location;

		switch (config.attributes[i].format) {
			case VertexFormat::FLOAT1:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32_SFLOAT;
				break;
			case VertexFormat::FLOAT2:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
				break;
			case VertexFormat::FLOAT3:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case VertexFormat::FLOAT4:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			default:
				TK_UNREACHABLE();
		}
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
	vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
	vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
	vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
	input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

	switch (config.options.primitive_topology) {
		case PrimitiveTopology::POINT_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case PrimitiveTopology::LINE_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case PrimitiveTopology::LINE_STRIP:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case PrimitiveTopology::TRIANGLE_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case PrimitiveTopology::TRIANGLE_STRIP:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		case PrimitiveTopology::TRIANGLE_FAN:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			break;
		case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::PATCH_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			break;
	}

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
	rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state_create_info.lineWidth = 1.0f;
	rasterization_state_create_info.depthBiasEnable = VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_state_create_info.depthBiasClamp = 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

	switch (config.options.front_face) {
		case FrontFace::CLOCKWISE:
			rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
			break;
		case FrontFace::COUNTER_CLOCKWISE:
			rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			break;
	}

	switch (config.options.polygon_mode) {
		case PolygonMode::FILL:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
			break;
		case PolygonMode::LINE:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE;
			break;
		case PolygonMode::POINT:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_POINT;
			break;
	}

	switch (config.options.cull_mode) {
		case CullMode::NONE:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
			break;
		case CullMode::FRONT:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
			break;
		case CullMode::BACK:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
			break;
		case CullMode::BOTH:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
			break;
	}

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
	multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.sampleShadingEnable = VK_FALSE;
	multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state_create_info.minSampleShading = 1.0f;
	multisample_state_create_info.pSampleMask = nullptr;
	multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_state_create_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
	depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.depthTestEnable = config.options.depth_test_enable ? VK_TRUE : VK_FALSE;
	depth_stencil_state_create_info.depthWriteEnable = config.options.depth_write_enable ? VK_TRUE : VK_FALSE;
	depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state_create_info.minDepthBounds = 0.0f;
	depth_stencil_state_create_info.maxDepthBounds = 1.0f;
	depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
	depth_stencil_state_create_info.front = {};
	depth_stencil_state_create_info.back = {};

	switch (config.options.depth_compare_op) {
		case CompareOp::NEVER:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NEVER;
			break;
		case CompareOp::LESS:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
			break;
		case CompareOp::EQUAL:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
			break;
		case CompareOp::LESS_OR_EQUAL:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;
		case CompareOp::GREATER:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER;
			break;
		case CompareOp::NOT_EQUAL:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
			break;
		case CompareOp::GREATER_OR_EQUAL:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
			break;
		case CompareOp::ALWAYS:
			depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
			break;
	}

	VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
	color_blend_attachment_state.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment_state.blendEnable = VK_TRUE;	 // TODO: enable blending
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	TempDynamicArray<VkPipelineColorBlendAttachmentState> color_blend_attachment_states;
	color_blend_attachment_states.emplace_back(color_blend_attachment_state);

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
	color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable = VK_FALSE;
	color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_state_create_info.attachmentCount = color_blend_attachment_states.size();
	color_blend_state_create_info.pAttachments = color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[0] = 1.0f;
	color_blend_state_create_info.blendConstants[1] = 1.0f;
	color_blend_state_create_info.blendConstants[2] = 1.0f;
	color_blend_state_create_info.blendConstants[3] = 1.0f;

	constexpr VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = ARRAY_SIZE(dynamic_states);
	dynamic_state_create_info.pDynamicStates = dynamic_states;

	VkViewport viewport{};
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	TempDynamicArray<VkFormat> formats(config.color_formats.size());
	for (u32 i = 0; i < formats.size(); i++) {
		formats[i] = map_color_format(config.color_formats[i]);
	}

	VkPipelineRenderingCreateInfoKHR pipeline_rendering_craete_info{};
	pipeline_rendering_craete_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipeline_rendering_craete_info.colorAttachmentCount = formats.size();
	pipeline_rendering_craete_info.pColorAttachmentFormats = formats.data();
	pipeline_rendering_craete_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	pipeline_rendering_craete_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	TK_ASSERT(m_resources.shader_layouts.exists(config.layout_handle));

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
	graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_create_info.stageCount = shader_stage_create_infos.size();
	graphics_pipeline_create_info.pStages = shader_stage_create_infos;
	graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
	graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
	graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
	graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
	graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
	graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;
	graphics_pipeline_create_info.subpass = 0;
	graphics_pipeline_create_info.layout = m_resources.shader_layouts[config.layout_handle].pipeline_layout;

	VulkanShader shader{};
	VkResult result = vkCreateGraphicsPipelines(
		m_state.device, nullptr, 1, &graphics_pipeline_create_info, m_state.allocation_callbacks, &shader.pipeline);

	for (u32 i = 0; i < shader_stage_create_infos.size(); i++) {
		vkDestroyShaderModule(m_state.device, shader_stage_create_infos[i].module, m_state.allocation_callbacks);
	}

	return shader;
}

VulkanShaderLayout VulkanBackend::create_vulkan_shader_layout(const ShaderLayoutConfig& config) {
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pSetLayouts = nullptr;

	VulkanShaderLayout shader_layout;
	vkCreatePipelineLayout(
		m_state.device, &pipeline_layout_create_info, m_state.allocation_callbacks, &shader_layout.pipeline_layout);

	return shader_layout;
}

toki::Expected<TempDynamicArray<toki::byte>, RendererErrors> VulkanBackend::compile_shader(
	ShaderStage stage, StringView source) {
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
		case renderer::ShaderStage::SHADER_STAGE_VERTEX:
			shader_kind = shaderc_shader_kind::shaderc_vertex_shader;
			break;
		case renderer::ShaderStage::SHADER_STAGE_FRAGMENT:
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
	toki::memcpy(bytes, compiled_data.data(), length);

	shaderc_result_release(result);
	shaderc_compile_options_release(options);
	shaderc_compiler_release(compiler);

	return compiled_data;
}

VkShaderModule VulkanBackend::create_shader_module(toki::Span<toki::byte> spirv) {
	VkShaderModuleCreateInfo shader_module_create_info{};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = spirv.size();
	shader_module_create_info.pCode = reinterpret_cast<const u32*>(spirv.data());

	VkShaderModule shader_module;
	VkResult result =
		vkCreateShaderModule(m_state.device, &shader_module_create_info, m_state.allocation_callbacks, &shader_module);
	TK_ASSERT(result == VK_SUCCESS);

	return shader_module;
}

}  // namespace toki::renderer
