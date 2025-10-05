#include "toki/renderer/private/vulkan/vulkan_commands.h"

#include <vulkan/vulkan_core.h>

#include "toki/core/common/assert.h"

namespace toki::renderer {

VulkanCommands::VulkanCommands(const VulkanState* state, VulkanCommandBuffer cmd): m_state(state), m_cmd(cmd) {}

void VulkanCommands::begin_pass() {
	VkRenderingAttachmentInfoKHR rendering_attachment_info{};
	rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	rendering_attachment_info.imageView = m_state->swapchain.get_current_image();
	rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	rendering_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
	rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.colorAttachmentCount = 1;
	rendering_info.pColorAttachments = &rendering_attachment_info;
	rendering_info.renderArea = VkRect2D{ { 0, 0 }, { 800, 600 } };
	rendering_info.layerCount = 1;
	vkCmdBeginRendering(m_cmd, &rendering_info);

	VkViewport viewport{};
	viewport.width = 800;
	viewport.height = 600;
	viewport.maxDepth = 1.0;
	vkCmdSetViewport(m_cmd, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { 800, 600 };
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

}  // namespace toki::renderer
