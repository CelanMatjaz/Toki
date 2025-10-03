#include "toki/renderer/private/vulkan/vulkan_commands.h"

#include <vulkan/vulkan_core.h>

namespace toki::renderer {

void VulkanCommands::begin_pass() {
	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	vkCmdBeginRendering(m_cmd, &rendering_info);
}

void VulkanCommands::end_pass() {}

void VulkanCommands::bind_shader(const ShaderHandle& handle) {}

void VulkanCommands::draw(u32 vertex_count) {
	vkCmdDraw(m_cmd, vertex_count, 1, 0, 0);
}

}  // namespace toki::renderer
