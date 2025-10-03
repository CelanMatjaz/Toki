#include <vulkan/vulkan_core.h>

#include "toki/renderer/private/vulkan/vulkan_backend.h"

namespace toki::renderer {

void VulkanBackend::frame_prepare() {}

void VulkanBackend::frame_cleanup() {
	vkEndCommandBuffer(m_resources.commands[0].m_cmd);
}

Commands* VulkanBackend::get_command_queue_for_frame() {
	m_resources.commands[0].m_cmd = m_resources.command_buffers[0].command_buffer;

	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(m_resources.commands[0].m_cmd, &command_buffer_begin_info);
	return &m_resources.commands[0];
}

}  // namespace toki::renderer
