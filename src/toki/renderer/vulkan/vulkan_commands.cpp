#include <toki/renderer/private/vulkan/vulkan_commands.h>
#include <toki/renderer/private/vulkan/vulkan_state.h>
#include <vulkan/vulkan.h>

#include "toki/renderer/commands.h"

namespace toki {

#define STATE reinterpret_cast<VulkanCommandsData*>(m_data)->state
#define CMD	  reinterpret_cast<VulkanCommandsData*>(m_data)->cmd

void Commands::begin_pass(const BeginPassConfig& config) {
	if (config.render_targets.size() == 0) {
		TK_ASSERT(
			config.swapchain_target_index.has_value(),
			"0 render targets not supported if swapchain target index is not provided");
	}

	VkRenderingInfo rendering_info{};
	rendering_info.sType	  = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea = VkRect2D{ { 0, 0 }, convert_to<VkExtent2D>(config.render_area_size) };
	rendering_info.layerCount = 1;

	u32 attachment_count = config.render_targets.size() > 0 ? config.render_targets.size() : 1;
	TempDynamicArray<VkRenderingAttachmentInfoKHR> rendering_attachments(attachment_count);
	for (u32 i = 0; i < rendering_attachments.size(); i++) {
		TK_ASSERT(
			i == config.swapchain_target_index.value_or(~static_cast<u32>(0)) ||
			STATE->textures.exists(config.render_targets[i]));
		VkRenderingAttachmentInfoKHR& rendering_attachment_info = rendering_attachments[i] = {};
		rendering_attachment_info.sType						  = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		rendering_attachment_info.imageLayout				  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		rendering_attachment_info.resolveMode				  = VK_RESOLVE_MODE_NONE;
		rendering_attachment_info.loadOp					  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		rendering_attachment_info.storeOp					  = VK_ATTACHMENT_STORE_OP_STORE;
		rendering_attachment_info.clearValue.color.float32[0] = 1.0f;
		rendering_attachment_info.clearValue.color.float32[1] = 1.0f;
		rendering_attachment_info.clearValue.color.float32[2] = 1.0f;
		rendering_attachment_info.clearValue.color.float32[3] = 1.0f;
		if (config.swapchain_target_index.has_value()) {
			rendering_attachment_info.imageView = STATE->swapchain.get_current_image().image_view();
		} else {
			rendering_attachment_info.imageView = STATE->textures.at(config.render_targets[i]).image_view();
		}
	}

	VkRenderingAttachmentInfoKHR depth_buffer_attachment_info{};
	if (config.depth_buffer.has_value()) {
		TK_ASSERT(STATE->textures.exists(config.depth_buffer.value()));
		depth_buffer_attachment_info.sType		 = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depth_buffer_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depth_buffer_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
		depth_buffer_attachment_info.loadOp		 = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_buffer_attachment_info.storeOp	 = VK_ATTACHMENT_STORE_OP_STORE;
		depth_buffer_attachment_info.imageView	 = STATE->textures.at(config.depth_buffer.value()).image_view();
		depth_buffer_attachment_info.clearValue.depthStencil = { 1.0f, 0 };
		rendering_info.pDepthAttachment						 = &depth_buffer_attachment_info;
	}

	rendering_info.colorAttachmentCount = rendering_attachments.size();
	rendering_info.pColorAttachments	= rendering_attachments.data();
	vkCmdBeginRendering(CMD, &rendering_info);

	VkViewport viewport{};
	viewport.width	  = config.render_area_size.x;
	viewport.height	  = config.render_area_size.y;
	viewport.maxDepth = 1.0;
	vkCmdSetViewport(CMD, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = convert_to<VkExtent2D>(config.render_area_size);
	vkCmdSetScissor(CMD, 0, 1, &scissor);
}

void Commands::end_pass() {
	vkCmdEndRendering(CMD);
}

void Commands::bind_shader(ShaderHandle handle) {
	TK_ASSERT(STATE->shaders.exists(handle));
	vkCmdBindPipeline(CMD, VK_PIPELINE_BIND_POINT_GRAPHICS, STATE->shaders.at(handle));
}

void Commands::draw(u32 vertex_count) {
	vkCmdDraw(CMD, vertex_count, 1, 0, 0);
}

void Commands::draw_indexed(u32 index_count) {
	vkCmdDrawIndexed(CMD, index_count, 1, 0, 0, 0);
}

void Commands::bind_index_buffer(BufferHandle handle) {
	TK_ASSERT(STATE->buffers.exists(handle));
	VulkanBuffer& buf = STATE->buffers.at(handle);
	vkCmdBindIndexBuffer(CMD, buf.buffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Commands::bind_vertex_buffer(BufferHandle handle) {
	TK_ASSERT(STATE->buffers.exists(handle));
	VulkanBuffer& buffer	= STATE->buffers.at(handle);
	VkBuffer buffer_array[] = { buffer.buffer() };
	VkDeviceSize offsets[]	= { 0 };
	vkCmdBindVertexBuffers(CMD, 0, 1, buffer_array, offsets);
}

void Commands::bind_uniforms(ShaderLayoutHandle handle) {
	TK_ASSERT(STATE->shader_layouts.exists(handle));
	VulkanShaderLayout& shader_layout = STATE->shader_layouts.at(handle);
	Span sets						  = shader_layout.sets();

	vkCmdBindDescriptorSets(
		CMD, VK_PIPELINE_BIND_POINT_GRAPHICS, shader_layout.layout(), 0, sets.size(), sets.data(), 0, nullptr);
}

}  // namespace toki
