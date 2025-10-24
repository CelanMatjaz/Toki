#include "toki/renderer/private/vulkan/vulkan_commands.h"

#include <vulkan/vulkan_core.h>

namespace toki {

VulkanCommands::VulkanCommands(const VulkanState* state, VulkanCommandBuffer cmd): m_state(state), m_cmd(cmd) {}

void VulkanCommands::begin_pass(const BeginPassConfig& config) {
	if (config.render_targets.size() == 0) {
		TK_ASSERT(
			config.swapchain_target_index.has_value(),
			"0 render targets not supported if swapchain target index is not provided");
	}

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea = VkRect2D{ { 0, 0 }, convert_to<VkExtent2D>(config.render_area_size) };
	rendering_info.layerCount = 1;

	u32 attachment_count = config.render_targets.size() > 0 ? config.render_targets.size() : 1;
	TempDynamicArray<VkRenderingAttachmentInfoKHR> rendering_attachments(attachment_count);
	for (u32 i = 0; i < rendering_attachments.size(); i++) {
		TK_ASSERT(
			i == config.swapchain_target_index.value_or(~static_cast<u32>(0)) ||
			m_state->textures.exists(config.render_targets[i]));
		VkRenderingAttachmentInfoKHR& rendering_attachment_info = rendering_attachments[i] = {};
		rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		rendering_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
		rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if (config.swapchain_target_index.has_value()) {
			rendering_attachment_info.imageView = m_state->swapchain.get_current_image().image_view();
		} else {
			rendering_attachment_info.imageView = m_state->textures.at(config.render_targets[i]).image_view();
		}
	}

	VkRenderingAttachmentInfoKHR depth_buffer_attachment_info{};
	if (config.depth_buffer.has_value()) {
		TK_ASSERT(m_state->textures.exists(config.depth_buffer.value()));
		depth_buffer_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth_buffer_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depth_buffer_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
		depth_buffer_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_buffer_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_buffer_attachment_info.imageView = m_state->textures.at(config.depth_buffer.value()).image_view();
		depth_buffer_attachment_info.clearValue.depthStencil = { 1.0f, 0 };
		rendering_info.pDepthAttachment = &depth_buffer_attachment_info;
	}

	rendering_info.colorAttachmentCount = rendering_attachments.size();
	rendering_info.pColorAttachments = rendering_attachments.data();
	vkCmdBeginRendering(m_cmd, &rendering_info);

	VkViewport viewport{};
	viewport.width = config.render_area_size.x;
	viewport.height = config.render_area_size.y;
	viewport.maxDepth = 1.0;
	vkCmdSetViewport(m_cmd, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = convert_to<VkExtent2D>(config.render_area_size);
	vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

void VulkanCommands::end_pass() {
	vkCmdEndRendering(m_cmd);
}

void VulkanCommands::bind_shader(ShaderHandle handle) {
	TK_ASSERT(m_state->shaders.exists(handle));
	vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_state->shaders.at(handle));
}

void VulkanCommands::draw(u32 vertex_count) {
	vkCmdDraw(m_cmd, vertex_count, 1, 0, 0);
}

void VulkanCommands::draw_indexed(u32 index_count) {
	vkCmdDrawIndexed(m_cmd, index_count, 1, 0, 0, 0);
}

void VulkanCommands::bind_index_buffer(BufferHandle handle) {
	TK_ASSERT(m_state->buffers.exists(handle));
	VulkanBuffer& buf = m_state->buffers.at(handle);
	vkCmdBindIndexBuffer(m_cmd, buf.buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommands::bind_vertex_buffer(BufferHandle handle) {
	TK_ASSERT(m_state->buffers.exists(handle));
	VulkanBuffer& buffer = m_state->buffers.at(handle);
	VkBuffer buffer_array[] = { buffer.buffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_cmd, 0, 1, buffer_array, offsets);
}

void VulkanCommands::bind_uniforms(ShaderLayoutHandle handle) {
	TK_ASSERT(m_state->shader_layouts.exists(handle));
	VulkanShaderLayout& shader_layout = m_state->shader_layouts.at(handle);
	Span sets = shader_layout.sets();

	vkCmdBindDescriptorSets(
		m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader_layout.layout(), 0, sets.size(), sets.data(), 0, nullptr);
}

}  // namespace toki
