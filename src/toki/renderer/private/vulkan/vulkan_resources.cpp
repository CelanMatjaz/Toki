#include "toki/renderer/private/vulkan/vulkan_resources.h"

#include <toki/core/common/assert.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/private/vulkan/vulkan_resources_utils.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>
#include <toki/renderer/private/vulkan/vulkan_utils.h>
#include <toki/renderer/types.h>
#include <vulkan/vulkan.h>

namespace toki {

VulkanSwapchain VulkanSwapchain::create(const VulkanSwapchainConfig& config, const VulkanState& state) {
	VulkanSwapchain swapchain{};

	swapchain.m_surface = create_surface(state, config.window);

	VkSurfaceCapabilitiesKHR surface_capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physical_device, swapchain.m_surface, &surface_capabilities);
	swapchain.m_surfaceCapabilities = surface_capabilities;

	swapchain.m_presentModes  = query_present_modes(state, swapchain.m_surface);
	swapchain.m_surfaceFormat = query_surface_formats(state, swapchain.m_surface);
	swapchain.m_extent		  = query_surface_extent(surface_capabilities, config.window);

	swapchain.recreate(state);

	return swapchain;
}

void VulkanSwapchain::destroy(const VulkanState& state) {
	for (u32 i = 0; i < m_images.size(); i++) {
		m_images[i].destroy(state);
	}
	vkDestroySwapchainKHR(state.logical_device, m_swapchain, state.allocation_callbacks);
	vkDestroySurfaceKHR(state.instance, m_surface, state.allocation_callbacks);
}

void VulkanSwapchain::recreate(const VulkanState& state) {
	VkSurfaceCapabilitiesKHR surface_properties;
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physical_device, m_surface, &surface_properties);
	TK_ASSERT(result == VK_SUCCESS);

	if (m_swapchain != VK_NULL_HANDLE && surface_properties.currentExtent.width == m_extent.width &&
		surface_properties.currentExtent.height == m_extent.height) {
		return;
	}

	m_images.clear();

	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType			   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface		   = m_surface;
	swapchain_create_info.minImageCount	   = m_surfaceCapabilities.minImageCount;
	swapchain_create_info.imageFormat	   = m_surfaceFormat.format;
	swapchain_create_info.imageColorSpace  = m_surfaceFormat.colorSpace;
	swapchain_create_info.imageExtent	   = m_extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage	   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.oldSwapchain	   = m_oldSwapchain;
	swapchain_create_info.preTransform	   = m_surfaceCapabilities.currentTransform;
	swapchain_create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.clipped		   = VK_TRUE;
	swapchain_create_info.presentMode =
		m_presentModes.array[!!static_cast<b8>(state.settings.flags & VSYNC_ENABLED & VSYNC_SUPPORTED)];

	if (state.indices[GRAPHICS_FAMILY_INDEX] == state.indices[PRESENT_FAMILY_INDEX]) {
		swapchain_create_info.imageSharingMode		= VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices	= state.indices;
	} else {
		swapchain_create_info.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices	= nullptr;
	}

	TK_LOG_INFO(
		"(Re)creating swapchain [{}x{}]",
		swapchain_create_info.imageExtent.width,
		swapchain_create_info.imageExtent.height);

	result =
		vkCreateSwapchainKHR(state.logical_device, &swapchain_create_info, state.allocation_callbacks, &m_swapchain);
	TK_ASSERT(result == VK_SUCCESS);

	if (m_oldSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(state.logical_device, m_oldSwapchain, state.allocation_callbacks);
	}

	m_oldSwapchain = m_swapchain;

	u32 swapchain_image_count{};
	vkGetSwapchainImagesKHR(state.logical_device, m_swapchain, &swapchain_image_count, nullptr);
	TempDynamicArray<VkImage> swapchain_images(swapchain_image_count);
	vkGetSwapchainImagesKHR(state.logical_device, m_swapchain, &swapchain_image_count, swapchain_images.data());

	m_images.resize(swapchain_image_count);

	for (u32 i = 0; i < swapchain_image_count; i++) {
		m_images[i] = WrappedVulkanTexture(state, swapchain_images[i], m_surfaceFormat.format);
	}
}

void VulkanSwapchain::acquire_next_image(const VulkanState& state) {
	if (m_resized) {
		recreate(state);
		m_resized = false;
	}

	VkResult result = vkAcquireNextImageKHR(
		state.logical_device,
		m_swapchain,
		U64_MAX,
		state.frames.get_image_available_semaphore(state).semaphore(),
		VK_NULL_HANDLE,
		&m_currentImageIndex);

	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || m_resized) {
		recreate(state);
		acquire_next_image(state);
		m_resized = false;
	} else {
		TK_ASSERT(result == VK_SUCCESS);
	}
}

VulkanTexture& VulkanSwapchain::get_current_image() const {
	return m_images[m_currentImageIndex].m_texture;
}

void VulkanSwapchain::window_listen_function(void*, void* listener, const Event& event) {
	VulkanSwapchain* swapchain = reinterpret_cast<VulkanSwapchain*>(listener);
	if (event.type() == EventType::WINDOW_RESIZE) {
		swapchain->m_resized = true;
		swapchain->m_extent	 = { static_cast<u32>(event.data().window.dimensions.x),
								 static_cast<u32>(event.data().window.dimensions.y) };
	}
}

VulkanShaderLayout VulkanShaderLayout::create(const ShaderLayoutConfig& config, const VulkanState& state) {
	VulkanShaderLayout shader_layout{};

	DescriptorSetLayoutConfig descriptor_set_layout_config{};
	for (u32 i = 0; i < config.uniform_sets.size(); i++) {
		descriptor_set_layout_config.uniforms = config.uniform_sets[i].uniform_configs;
		shader_layout.create_descriptor_set_layout(state, descriptor_set_layout_config);
	}

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType				   = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges	   = nullptr;
	pipeline_layout_create_info.setLayoutCount		   = static_cast<u32>(shader_layout.m_descriptorSetLayouts.size());
	pipeline_layout_create_info.pSetLayouts			   = shader_layout.m_descriptorSetLayouts.data();

	vkCreatePipelineLayout(
		state.logical_device,
		&pipeline_layout_create_info,
		state.allocation_callbacks,
		&shader_layout.m_pipelineLayout);

	AllocateDescriptorSetConfig allocate_descriptor_set_config{};
	allocate_descriptor_set_config.descriptor_pool = state.descriptor_pool.descriptor_pool();
	shader_layout.allocate_descriptor_sets(state, allocate_descriptor_set_config);

	return shader_layout;
}

void VulkanShaderLayout::destroy(const VulkanState& state) {
	for (u32 i = 0; i < m_descriptorSetLayouts.size(); i++) {
		vkDestroyDescriptorSetLayout(state.logical_device, m_descriptorSetLayouts[i], state.allocation_callbacks);
	}
	vkDestroyPipelineLayout(state.logical_device, m_pipelineLayout, state.allocation_callbacks);
}

void VulkanShaderLayout::create_descriptor_set_layout(
	const VulkanState& state, const DescriptorSetLayoutConfig& config) {
	TempDynamicArray<VkDescriptorSetLayoutBinding> layout_bindinds(config.uniforms.size());

	for (u32 i = 0; i < config.uniforms.size(); i++) {
		VkDescriptorSetLayoutBinding& descriptor_set_layout_binding = layout_bindinds[i] = {};
		descriptor_set_layout_binding.binding											 = config.uniforms[i].binding;
		descriptor_set_layout_binding.descriptorType  = get_descriptor_type(config.uniforms[i].type);
		descriptor_set_layout_binding.descriptorCount = config.uniforms[i].count;
		descriptor_set_layout_binding.stageFlags	  = get_shader_stage_flags(config.uniforms[i].shader_stage_flags);
		descriptor_set_layout_binding.pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
	descriptor_set_layout_info.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_info.bindingCount = static_cast<u32>(layout_bindinds.size());
	descriptor_set_layout_info.pBindings	= layout_bindinds.data();
	descriptor_set_layout_info.flags		= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	VkDescriptorSetLayout descriptor_set_layout;
	VkResult result = vkCreateDescriptorSetLayout(
		state.logical_device, &descriptor_set_layout_info, state.allocation_callbacks, &descriptor_set_layout);
	TK_ASSERT(result == VK_SUCCESS);

	m_descriptorSetLayouts.push_back(descriptor_set_layout);
}

void VulkanShaderLayout::allocate_descriptor_sets(const VulkanState& state, const AllocateDescriptorSetConfig& config) {
	m_descriptorSets.resize(m_descriptorSetLayouts.size());

	VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
	descriptor_set_allocate_info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_info.descriptorPool		= config.descriptor_pool;
	descriptor_set_allocate_info.descriptorSetCount = m_descriptorSetLayouts.size();
	descriptor_set_allocate_info.pSetLayouts		= m_descriptorSetLayouts.data();

	VkResult result =
		vkAllocateDescriptorSets(state.logical_device, &descriptor_set_allocate_info, m_descriptorSets.data());
	TK_ASSERT(result == VK_SUCCESS);
}

void VulkanShaderLayout::set_descriptors(const VulkanState& state, const SetUniformConfig& config) {
	TempDynamicArray<VkWriteDescriptorSet> writes(config.uniforms.size());
	TempDynamicArray<VkDescriptorBufferInfo> descriptor_buffer_infos;
	descriptor_buffer_infos.reserve(config.uniforms.size());
	TempDynamicArray<VkDescriptorImageInfo> descriptor_image_infos;
	descriptor_image_infos.reserve(config.uniforms.size());

	for (u32 i = 0; i < writes.size(); i++) {
		writes[i]				  = {};
		writes[i].sType			  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet		  = m_descriptorSets[config.uniforms[i].set_index];
		writes[i].dstBinding	  = config.uniforms[i].binding;
		writes[i].dstArrayElement = config.uniforms[i].array_element;
		writes[i].descriptorCount = 1;

		switch (config.uniforms[i].type) {
			case UniformType::UNIFORM_BUFFER: {
				TK_ASSERT(state.buffers.exists(config.uniforms[i].handle.uniform_buffer));
				VulkanBuffer buffer = state.buffers.at(config.uniforms[i].handle.uniform_buffer);

				VkDescriptorBufferInfo descriptor_buffer_info{};
				descriptor_buffer_info.buffer = buffer.buffer();
				descriptor_buffer_info.offset = 0;
				descriptor_buffer_info.range  = buffer.size();
				descriptor_buffer_infos.push_back(descriptor_buffer_info);

				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writes[i].pBufferInfo	 = &descriptor_buffer_infos.last();
			} break;

			case UniformType::TEXTURE: {
				TK_ASSERT(state.textures.exists(config.uniforms[i].handle.texture));
				VulkanTexture texture = state.textures.at(config.uniforms[i].handle.texture);

				VkDescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.imageView = texture.image_view();
				descriptor_image_infos.push_back(descriptor_image_info);

				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				writes[i].pImageInfo	 = &descriptor_image_infos.last();
			} break;

			case UniformType::SAMPLER: {
				TK_ASSERT(state.samplers.exists(config.uniforms[i].handle.sampler));
				VulkanSampler sampler = state.samplers.at(config.uniforms[i].handle.sampler);

				VkDescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.sampler = sampler;
				descriptor_image_infos.push_back(descriptor_image_info);

				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				writes[i].pImageInfo	 = &descriptor_image_infos.last();
			} break;

			case UniformType::TEXTURE_WITH_SAMPLER: {
				TextureHandle texture_handle = config.uniforms[i].handle.texture_with_sampler.texture;
				TK_ASSERT(state.textures.exists(texture_handle));
				VulkanTexture texture = state.textures.at(texture_handle);

				SamplerHandle sampler_handle = config.uniforms[i].handle.texture_with_sampler.sampler;
				TK_ASSERT(state.samplers.exists(sampler_handle));
				VulkanSampler sampler = state.samplers.at(sampler_handle);

				VkDescriptorImageInfo descriptor_image_info{};
				descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_info.imageView	  = texture.image_view();
				descriptor_image_info.sampler	  = sampler;
				descriptor_image_infos.push_back(descriptor_image_info);

				writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writes[i].pImageInfo	 = &descriptor_image_infos.last();
			} break;
		}
	}

	vkUpdateDescriptorSets(state.logical_device, writes.size(), writes.data(), 0, nullptr);
}

VulkanShader VulkanShader::create(const ShaderConfig& config, const VulkanState& state) {
	TK_ASSERT(config.color_formats.size() > 0);

	TempDynamicArray<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
	shader_stage_create_infos.reserve(ShaderStageFlags::SHADER_STAGE_SIZE);

	for (u32 i = 0; i < config.sources.size(); i++) {
		if (config.sources[i].size() == 0) {
			continue;
		}

		auto compile_shader_result = compile_shader(static_cast<ShaderStageFlags>(i), config.sources[i]);
		TK_ASSERT(compile_shader_result);

		shader_stage_create_infos.push_back({});
		VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_stage_create_infos.last();

		shader_stage_create_info.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_create_info.module = create_shader_module(state, compile_shader_result.value());
		shader_stage_create_info.pName	= "main";

		switch (static_cast<ShaderStageFlags>(i)) {
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
		vertex_binding_descriptions[i].stride  = config.bindings[i].stride;

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
		vertex_attribute_descriptions[i].binding  = config.attributes[i].binding;
		vertex_attribute_descriptions[i].offset	  = config.attributes[i].offset;
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
	vertex_input_state_create_info.vertexBindingDescriptionCount   = vertex_binding_descriptions.size();
	vertex_input_state_create_info.pVertexBindingDescriptions	   = vertex_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
	vertex_input_state_create_info.pVertexAttributeDescriptions	   = vertex_attribute_descriptions.data();

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
	rasterization_state_create_info.sType			 = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state_create_info.lineWidth				= 1.0f;
	rasterization_state_create_info.depthBiasEnable			= VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_state_create_info.depthBiasClamp			= 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor	= 0.0f;

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
	multisample_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.sampleShadingEnable	= VK_FALSE;
	multisample_state_create_info.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisample_state_create_info.minSampleShading		= 1.0f;
	multisample_state_create_info.pSampleMask			= nullptr;
	multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_state_create_info.alphaToOneEnable		= VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
	depth_stencil_state_create_info.sType				  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.depthTestEnable		  = config.options.depth_test_enable ? VK_TRUE : VK_FALSE;
	depth_stencil_state_create_info.depthWriteEnable	  = config.options.depth_write_enable ? VK_TRUE : VK_FALSE;
	depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state_create_info.minDepthBounds		  = 0.0f;
	depth_stencil_state_create_info.maxDepthBounds		  = 1.0f;
	depth_stencil_state_create_info.stencilTestEnable	  = VK_FALSE;
	depth_stencil_state_create_info.front				  = {};
	depth_stencil_state_create_info.back				  = {};

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
	color_blend_attachment_state.blendEnable		 = config.options.enable_blending ? VK_TRUE : VK_FALSE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.colorBlendOp		 = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.alphaBlendOp		 = VK_BLEND_OP_ADD;

	TempDynamicArray<VkPipelineColorBlendAttachmentState> color_blend_attachment_states(
		config.color_formats.size(), color_blend_attachment_state);

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
	color_blend_state_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable		= VK_FALSE;
	color_blend_state_create_info.logicOp			= VK_LOGIC_OP_COPY;
	color_blend_state_create_info.attachmentCount	= color_blend_attachment_states.size();
	color_blend_state_create_info.pAttachments		= color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[0] = 1.0f;
	color_blend_state_create_info.blendConstants[1] = 1.0f;
	color_blend_state_create_info.blendConstants[2] = 1.0f;
	color_blend_state_create_info.blendConstants[3] = 1.0f;

	constexpr VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
	dynamic_state_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = ARRAY_SIZE(dynamic_states);
	dynamic_state_create_info.pDynamicStates	= dynamic_states;

	VkViewport viewport{};
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType		 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports	 = &viewport;
	viewport_state_create_info.scissorCount	 = 1;
	viewport_state_create_info.pScissors	 = &scissor;

	TempDynamicArray<VkFormat> formats(config.color_formats.size());
	for (u32 i = 0; i < formats.size(); i++) {
		formats[i] = map_color_format(config.color_formats[i]);
	}
	formats[0] = state.swapchain;

	VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
	pipeline_rendering_create_info.sType				   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipeline_rendering_create_info.colorAttachmentCount	   = formats.size();
	pipeline_rendering_create_info.pColorAttachmentFormats = formats.data();
	pipeline_rendering_create_info.depthAttachmentFormat =
		map_color_format(config.depth_format.value_or(ColorFormat::NONE));
	pipeline_rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
	graphics_pipeline_create_info.sType				  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_create_info.pNext				  = &pipeline_rendering_create_info;
	graphics_pipeline_create_info.stageCount		  = shader_stage_create_infos.size();
	graphics_pipeline_create_info.pStages			  = shader_stage_create_infos.data();
	graphics_pipeline_create_info.pVertexInputState	  = &vertex_input_state_create_info;
	graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
	graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	graphics_pipeline_create_info.pMultisampleState	  = &multisample_state_create_info;
	graphics_pipeline_create_info.pColorBlendState	  = &color_blend_state_create_info;
	graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_create_info;
	graphics_pipeline_create_info.pViewportState	  = &viewport_state_create_info;
	graphics_pipeline_create_info.pDynamicState		  = &dynamic_state_create_info;
	graphics_pipeline_create_info.renderPass		  = VK_NULL_HANDLE;
	graphics_pipeline_create_info.subpass			  = 0;
	graphics_pipeline_create_info.layout			  = state.shader_layouts[config.layout_handle].m_pipelineLayout;

	VulkanShader shader{};
	VkResult result = vkCreateGraphicsPipelines(
		state.logical_device,
		nullptr,
		1,
		&graphics_pipeline_create_info,
		state.allocation_callbacks,
		&shader.m_pipeline);
	TK_ASSERT(result == VK_SUCCESS);

	for (u32 i = 0; i < shader_stage_create_infos.size(); i++) {
		vkDestroyShaderModule(state.logical_device, shader_stage_create_infos[i].module, state.allocation_callbacks);
	}

	return shader;
}

void VulkanShader::destroy(const VulkanState& state) {
	vkDestroyPipeline(state.logical_device, m_pipeline, state.allocation_callbacks);
}

VulkanBuffer VulkanBuffer::create(const VulkanBufferConfig& config, const VulkanState& state) {
	VulkanBuffer buffer{};
	buffer.m_size = config.buffer_config.size;
	buffer.m_type = config.buffer_config.type;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size	 = config.buffer_config.size;
	bufferInfo.usage =
		config.override_usage != 0 ? config.override_usage : get_buffer_usage_flags(config.buffer_config.type);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(state.logical_device, &bufferInfo, state.allocation_callbacks, &buffer.m_buffer);
	TK_ASSERT(result == VK_SUCCESS);

	MemoryAllocateConfig device_allocate_config{};
	device_allocate_config.memory_property_flags =
		config.override_memory_properties != 0
			? config.override_memory_properties
			: static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkGetBufferMemoryRequirements(state.logical_device, buffer.m_buffer, &device_allocate_config.memory_requirements);
	buffer.m_deviceMemory = allocate_device_memory(device_allocate_config, state);

	result = vkBindBufferMemory(state.logical_device, buffer.m_buffer, buffer.m_deviceMemory, 0);
	TK_ASSERT(result == VK_SUCCESS);

	return buffer;
}

void VulkanBuffer::destroy(const VulkanState& state) {
	vkDestroyBuffer(state.logical_device, m_buffer, state.allocation_callbacks);
	vkFreeMemory(state.logical_device, m_deviceMemory, state.allocation_callbacks);
}

void VulkanBuffer::set_data(const VulkanState& state, const void* data, u64 size) {
	void* mapped_memory = map_memory(state);
	toki::memcpy(mapped_memory, data, size);
	unmap_memory(state);
}

void* VulkanBuffer::map_memory(const VulkanState& state) {
	void* data;
	VkResult result = vkMapMemory(state.logical_device, m_deviceMemory, 0, m_size, 0, &data);
	TK_ASSERT(result == VK_SUCCESS);
	return data;
}

void VulkanBuffer::unmap_memory(const VulkanState& state) {
	vkUnmapMemory(state.logical_device, m_deviceMemory);
}

void VulkanBuffer::copy_to_buffer(
	VulkanCommandBuffer cmd, const VulkanBufferCopyConfig& dst_buffer_copy_config, u64 self_offset) const {
	VkBufferCopy buffer_copy{};
	buffer_copy.size	  = dst_buffer_copy_config.size;
	buffer_copy.dstOffset = dst_buffer_copy_config.offset;
	buffer_copy.srcOffset = self_offset;
	vkCmdCopyBuffer(cmd, m_buffer, dst_buffer_copy_config.buffer, 1, &buffer_copy);
}

void VulkanBuffer::copy_to_image(
	VulkanCommandBuffer cmd, const VulkanBufferImageCopyConfig& dst_image_copy_config, u32 self_offset) const {
	VkBufferImageCopy buffer_image_copy{};
	buffer_image_copy.bufferOffset					  = self_offset;
	buffer_image_copy.bufferRowLength				  = 0;
	buffer_image_copy.bufferImageHeight				  = 0;
	buffer_image_copy.imageSubresource.aspectMask	  = VK_IMAGE_ASPECT_COLOR_BIT;
	buffer_image_copy.imageSubresource.mipLevel		  = 0;
	buffer_image_copy.imageSubresource.baseArrayLayer = 0;
	buffer_image_copy.imageSubresource.layerCount	  = 1;
	buffer_image_copy.imageOffset					  = VkOffset3D{ 0, 0, 0 };
	buffer_image_copy.imageExtent = VkExtent3D{ dst_image_copy_config.width, dst_image_copy_config.height, 1 };

	vkCmdCopyBufferToImage(
		cmd, m_buffer, dst_image_copy_config.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);
}

VulkanTexture VulkanTexture::create(const TextureConfig& config, const VulkanState& state) {
	VulkanTexture texture{};

	texture.m_metadata = config;

	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	switch (config.format) {
		case ColorFormat::R8:
			format = VK_FORMAT_R8_UNORM;
			break;
		case ColorFormat::RGBA8:
			format = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case ColorFormat::DEPTH_STENCIL:
			format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			break;
		case ColorFormat::COLOR_FORMAT_COUNT:
		case ColorFormat::NONE:
			TK_UNREACHABLE();
	}

	VkImageCreateInfo image_create_info{};
	image_create_info.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType		= VK_IMAGE_TYPE_2D;
	image_create_info.extent.width	= static_cast<uint32_t>(config.width);
	image_create_info.extent.height = static_cast<uint32_t>(config.height);
	image_create_info.extent.depth	= 1;
	image_create_info.mipLevels		= 1;
	image_create_info.arrayLayers	= 1;
	image_create_info.format		= format;
	image_create_info.tiling		= VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage			= get_image_usage_flags(config.flags);
	image_create_info.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.samples		= VK_SAMPLE_COUNT_1_BIT;

	VkResult result =
		vkCreateImage(state.logical_device, &image_create_info, state.allocation_callbacks, &texture.m_image);
	TK_ASSERT(result == VK_SUCCESS);

	MemoryAllocateConfig device_allocate_config{};
	vkGetImageMemoryRequirements(state.logical_device, texture.m_image, &device_allocate_config.memory_requirements);
	device_allocate_config.memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	texture.m_deviceMemory						 = allocate_device_memory(device_allocate_config, state);

	result = vkBindImageMemory(state.logical_device, texture.m_image, texture.m_deviceMemory, 0);
	TK_ASSERT(result == VK_SUCCESS);

	ImageViewConfig image_view_config{};
	image_view_config.image	 = texture.m_image;
	image_view_config.format = format;
	texture.m_imageView		 = create_image_view(image_view_config, state);

	return texture;
}

void VulkanTexture::destroy(const VulkanState& state) {
	vkDestroyImageView(state.logical_device, m_imageView, state.allocation_callbacks);
	vkDestroyImage(state.logical_device, m_image, state.allocation_callbacks);
	vkFreeMemory(state.logical_device, m_deviceMemory, state.allocation_callbacks);
}

void VulkanTexture::transition_layout(VulkanCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout) {
	VkImageMemoryBarrier barrier{};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= old_layout;
	barrier.newLayout						= new_layout;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= m_image;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	VkPipelineStageFlags src_stage = 0;
	VkPipelineStageFlags dst_stage = 0;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage			  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage			  = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (
		old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage			  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage			  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (
		old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src_stage			  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage			  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src_stage			  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage			  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} else if (
		old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;
		src_stage			  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dst_stage			  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	} else {
		TK_ASSERT(false);
	}

	vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

WrappedVulkanTexture::WrappedVulkanTexture(const VulkanState& state, VkImage image, VkFormat format) {
	m_texture.m_image = image;

	ImageViewConfig image_view_config{};
	image_view_config.image	 = image;
	image_view_config.format = format;
	m_texture.m_imageView	 = create_image_view(image_view_config, state);
}

void WrappedVulkanTexture::destroy(const VulkanState& state) {
	vkDestroyImageView(state.logical_device, m_texture.m_imageView, state.allocation_callbacks);
}

VulkanSampler VulkanSampler::create(const SamplerConfig& config, const VulkanState& state) {
	VulkanSampler sampler{};

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(state.physical_device, &properties);

	VkSamplerCreateInfo sampler_create_info{};
	sampler_create_info.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.magFilter				= get_filter(config.mag_filter);
	sampler_create_info.minFilter				= get_filter(config.mag_filter);
	sampler_create_info.addressModeU			= get_address_mode(config.address_mode_u);
	sampler_create_info.addressModeV			= get_address_mode(config.address_mode_v);
	sampler_create_info.addressModeW			= get_address_mode(config.address_mode_w);
	sampler_create_info.compareEnable			= VK_FALSE;
	sampler_create_info.compareOp				= VK_COMPARE_OP_ALWAYS;
	sampler_create_info.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.mipLodBias				= 0.0f;
	sampler_create_info.minLod					= 0.0f;
	sampler_create_info.maxLod					= 0.0f;
	sampler_create_info.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_create_info.unnormalizedCoordinates = !config.use_normalized_coords;
	sampler_create_info.anisotropyEnable		= VK_TRUE;
	sampler_create_info.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;

	VkResult result =
		vkCreateSampler(state.logical_device, &sampler_create_info, state.allocation_callbacks, &sampler.m_sampler);
	TK_ASSERT(result == VK_SUCCESS);

	return sampler;
}

void VulkanSampler::destroy(const VulkanState& state) {
	vkDestroySampler(state.logical_device, m_sampler, state.allocation_callbacks);
}

VulkanStagingBuffer VulkanStagingBuffer::create(const StagingBufferConfig& config, const VulkanState& state) {
	VulkanStagingBuffer staging_buffer{};

	VulkanBufferConfig buffer_config{};
	buffer_config.buffer_config.size = config.size;
	buffer_config.override_usage	 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_config.override_memory_properties =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	staging_buffer.m_buffer = VulkanBuffer::create(buffer_config, state);

	staging_buffer.m_mappedMemory = staging_buffer.m_buffer.map_memory(state);

	return staging_buffer;
}

void VulkanStagingBuffer::destroy(const VulkanState& state) {
	m_buffer.unmap_memory(state);
	m_buffer.destroy(state);
}

void VulkanStagingBuffer::set_data_for_buffer(
	const VulkanState& state, VulkanBuffer& dst_buffer, const void* data, u64 size) {
	TK_ASSERT(data != nullptr && size != 0);

	// Copy data to own memory
	toki::memcpy(&reinterpret_cast<byte*>(m_mappedMemory)[m_offset], data, size);

	// Copy just copied data to destination buffer
	VulkanBufferCopyConfig dst_buffer_copy_config{};
	dst_buffer_copy_config.buffer = dst_buffer.m_buffer;
	dst_buffer_copy_config.offset = 0;
	dst_buffer_copy_config.size	  = size;

	{
		VulkanCommandBuffer cmd = state.temporary_command_pool.begin_single_time_submit_command_buffer(state);
		m_buffer.copy_to_buffer(cmd, dst_buffer_copy_config, m_offset);
		state.temporary_command_pool.submit_single_time_submit_command_buffer(state, cmd);
	}

	m_offset += size;
}

void VulkanStagingBuffer::set_data_for_image(
	const VulkanState& state, VulkanTexture& dst_texture, const void* data, u64 size) {
	TK_ASSERT(data != nullptr && size != 0);

	toki::memcpy(&reinterpret_cast<byte*>(m_mappedMemory)[m_offset], data, size);

	VulkanBufferImageCopyConfig dst_image_copy_config{};
	dst_image_copy_config.image	 = dst_texture.image();
	dst_image_copy_config.width	 = dst_texture.width();
	dst_image_copy_config.height = dst_texture.height();

	{
		VulkanCommandBuffer cmd = state.temporary_command_pool.begin_single_time_submit_command_buffer(state);
		dst_texture.transition_layout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_buffer.copy_to_image(cmd, dst_image_copy_config, m_offset);
		dst_texture.transition_layout(
			cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		state.temporary_command_pool.submit_single_time_submit_command_buffer(state, cmd);
	}

	m_offset += size;
}

void VulkanStagingBuffer::reset() {
	m_offset = 0;
}

void VulkanCommandBuffer::begin(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = flags;
	VkResult result					= vkBeginCommandBuffer(m_commandBuffer, &command_buffer_begin_info);
	TK_ASSERT(result == VK_SUCCESS);
	this->m_state = CommandBufferState::RECORDING;
}

void VulkanCommandBuffer::end() {
	VkResult result = vkEndCommandBuffer(m_commandBuffer);
	TK_ASSERT(result == VK_SUCCESS);
	this->m_state = CommandBufferState::EXECUTABLE;
}

void VulkanCommandBuffer::reset() {
	VkResult result = vkResetCommandBuffer(m_commandBuffer, 0);
	TK_ASSERT(result == VK_SUCCESS);
	this->m_state = CommandBufferState::INITIAL;
}

void VulkanCommandBuffer::free(const VulkanState& state) {
	vkFreeCommandBuffers(state.logical_device, state.command_pool.command_pool(), 1, &m_commandBuffer);
}

VulkanCommandPool VulkanCommandPool::create(
	[[maybe_unused]] const CommandPoolConfig& config, const VulkanState& state) {
	VulkanCommandPool command_pool{};

	VkCommandPoolCreateInfo command_pool_create_info{};
	command_pool_create_info.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex = state.indices[GRAPHICS_FAMILY_INDEX];

	VkResult result = vkCreateCommandPool(
		state.logical_device, &command_pool_create_info, state.allocation_callbacks, &command_pool.m_commandPool);
	TK_ASSERT(result == VK_SUCCESS);

	return command_pool;
}

void VulkanCommandPool::destroy(const VulkanState& state) {
	vkDestroyCommandPool(state.logical_device, m_commandPool, state.allocation_callbacks);
}

PersistentDynamicArray<VulkanCommandBuffer> VulkanCommandPool::allocate_command_buffers(
	const VulkanState& state, u32 count, VkCommandBufferLevel level) const {
	VkCommandBufferAllocateInfo command_buffer_allocate_info{};
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool		= m_commandPool;
	command_buffer_allocate_info.level				= level;
	command_buffer_allocate_info.commandBufferCount = count;

	TempDynamicArray<VkCommandBuffer> command_buffers(count);
	VkResult result =
		vkAllocateCommandBuffers(state.logical_device, &command_buffer_allocate_info, command_buffers.data());
	TK_ASSERT(result == VK_SUCCESS);

	PersistentDynamicArray<VulkanCommandBuffer> command_buffers_out(count);
	for (u32 i = 0; i < count; i++) {
		command_buffers_out[i].m_commandBuffer = command_buffers[i];
		command_buffers_out[i].m_state		   = CommandBufferState::INITIAL;
	}

	return command_buffers_out;
}

void VulkanCommandPool::free_command_buffers(
	const VulkanState& state, toki::Span<VulkanCommandBuffer> command_buffers) const {
	TK_ASSERT(command_buffers.size() > 0);
	TempDynamicArray<VkCommandBuffer> to_free_command_buffers(command_buffers.size());

	for (u32 i = 0; i < command_buffers.size(); i++) {
		to_free_command_buffers[i] = command_buffers[i].m_commandBuffer;
	}

	vkFreeCommandBuffers(state.logical_device, m_commandPool, command_buffers.size(), to_free_command_buffers.data());
}

VulkanCommandBuffer VulkanCommandPool::begin_single_time_submit_command_buffer(const VulkanState& state) const {
	auto command_buffers = allocate_command_buffers(state, 1);
	command_buffers[0].begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	return command_buffers[0];
}

void VulkanCommandPool::submit_single_time_submit_command_buffer(
	const VulkanState& state, VulkanCommandBuffer command_buffer) const {
	command_buffer.end();

	VkSubmitInfo submit_info{};
	submit_info.sType			   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers	   = &command_buffer.m_commandBuffer;

	vkQueueSubmit(state.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(state.graphics_queue);

	VulkanCommandBuffer command_buffers[] = { command_buffer };
	free_command_buffers(state, command_buffers);
}

VulkanFrames VulkanFrames::create(VulkanState& state) {
	VulkanFrames vulkan_frames{};

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vulkan_frames.m_inFlightFenceHandles[i] = { state.fences.emplace_at_first(VulkanFence::create(true, state)) };
		vulkan_frames.m_imageAvailableSemaphoreHandles[i] = { state.semaphores.emplace_at_first(
			VulkanSemaphore::create({ .stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, state)) };
		vulkan_frames.m_renderFinishedSemaphoreHandles[i] = { state.semaphores.emplace_at_first(
			VulkanSemaphore::create({ .stage_flags = 0 }, state)) };
	}

	return vulkan_frames;
}

void VulkanFrames::destroy(const VulkanState& state) {
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		state.fences.at(m_inFlightFenceHandles[i]).destroy(state);
		state.semaphores.at(m_imageAvailableSemaphoreHandles[i]).destroy(state);
		state.semaphores.at(m_renderFinishedSemaphoreHandles[i]).destroy(state);
	}
}

void VulkanFrames::frame_prepare(VulkanState& state) {
	VkFence fence = get_in_flight_fence(state).fence();

	VkResult result = vkWaitForFences(state.logical_device, 1, &fence, VK_TRUE, U64_MAX);
	TK_ASSERT(result == VK_SUCCESS);

	state.swapchain.acquire_next_image(state);

	result = vkResetFences(state.logical_device, 1, &fence);
	TK_ASSERT(result == VK_SUCCESS);
}

void VulkanFrames::frame_cleanup(VulkanState& state) {
	vkDeviceWaitIdle(state.logical_device);
	increment_frame();
}

VulkanSemaphore VulkanFrames::get_image_available_semaphore(const VulkanState& state) const {
	return state.semaphores.at(m_imageAvailableSemaphoreHandles[m_currentFrame]);
}

VulkanSemaphore VulkanFrames::get_render_finished_semaphore(const VulkanState& state) const {
	return state.semaphores.at(m_renderFinishedSemaphoreHandles[m_currentFrame]);
}

VulkanFence VulkanFrames::get_in_flight_fence(const VulkanState& state) const {
	return state.fences.at(m_inFlightFenceHandles[m_currentFrame]);
}

SemaphoreHandle VulkanFrames::get_image_available_semaphore_handle() const {
	return m_imageAvailableSemaphoreHandles[m_currentFrame];
}

SemaphoreHandle VulkanFrames::get_render_finished_semaphore_handle() const {
	return m_renderFinishedSemaphoreHandles[m_currentFrame];
}

FenceHandle VulkanFrames::get_in_flight_fence_handle() const {
	return m_inFlightFenceHandles[m_currentFrame];
}

void VulkanFrames::increment_frame() {
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VulkanDescriptorPool VulkanDescriptorPool::create(const DescriptorPoolConfig& config, const VulkanState& state) {
	VulkanDescriptorPool descriptor_pool{};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
	descriptor_pool_create_info.sType		  = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.poolSizeCount = static_cast<u32>(config.pool_sizes.size());
	descriptor_pool_create_info.pPoolSizes	  = config.pool_sizes.data();
	descriptor_pool_create_info.maxSets		  = config.max_sets;
	descriptor_pool_create_info.flags		  = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	VkResult result = vkCreateDescriptorPool(
		state.logical_device,
		&descriptor_pool_create_info,
		state.allocation_callbacks,
		&descriptor_pool.m_descriptorPool);
	TK_ASSERT(result == VK_SUCCESS);

	return descriptor_pool;
}

void VulkanDescriptorPool::destroy(const VulkanState& state) {
	vkDestroyDescriptorPool(state.logical_device, m_descriptorPool, state.allocation_callbacks);
}

VulkanFence VulkanFence::create(b8 signaled, const VulkanState& state) {
	VulkanFence fence{};

	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	VkResult result =
		vkCreateFence(state.logical_device, &fence_create_info, state.allocation_callbacks, &fence.m_fence);
	TK_ASSERT(result == VK_SUCCESS);

	return fence;
}

void VulkanFence::destroy(const VulkanState& state) {
	vkDestroyFence(state.logical_device, m_fence, state.allocation_callbacks);
}

void VulkanFence::wait(const VulkanState& state, u64 timeout) {
	vkWaitForFences(state.logical_device, 1, &m_fence, VK_TRUE, timeout);
}

void VulkanFence::reset(const VulkanState& state) {
	vkResetFences(state.logical_device, 1, &m_fence);
}

VulkanSemaphore VulkanSemaphore::create(const SemaphoreConfig& config, const VulkanState& state) {
	VulkanSemaphore semaphore{};
	semaphore.m_stageFlags = config.stage_flags;

	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.flags = 0;
	VkResult result				= vkCreateSemaphore(
		state.logical_device, &semaphore_create_info, state.allocation_callbacks, &semaphore.m_semaphore);
	TK_ASSERT(result == VK_SUCCESS);

	return semaphore;
}

void VulkanSemaphore::destroy(const VulkanState& state) {
	vkDestroySemaphore(state.logical_device, m_semaphore, state.allocation_callbacks);
}

}  // namespace toki
