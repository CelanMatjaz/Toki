#include "toki/core/common/assert.h"
#include "toki/renderer/types.h"
#include "toki/renderer/private/vulkan/vulkan_backend.h"

namespace toki::renderer {

PersistentDynamicArray<CommandBuffer> VulkanBackend::allocate_command_buffers(u32 count, VkCommandPool command_pool) {
	VkCommandBufferAllocateInfo command_buffer_allocate_info{};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = count;

	TempDynamicArray<VkCommandBuffer> command_buffers(count);
	VkResult result = vkAllocateCommandBuffers(m_state.device, &command_buffer_allocate_info, command_buffers.data());
	TK_ASSERT(result == VK_SUCCESS);

	PersistentDynamicArray<CommandBuffer> command_buffers_out(count);
	for (u32 i = 0; i < count; i++) {
		command_buffers_out[i].command_buffer = command_buffers[i];
		command_buffers_out[i].state = CommandBufferState::INITIAL;
	}

	return command_buffers_out;
}

void command_buffer_reset(CommandBuffer& cmd) {
	VkResult result = vkResetCommandBuffer(cmd.command_buffer, 0);
	TK_ASSERT(result);
	cmd.state = CommandBufferState::INITIAL;
}

void command_buffer_begin(CommandBuffer cmd) {
	VkResult result = vkResetCommandBuffer(cmd.command_buffer, 0);
	TK_ASSERT(result);
	cmd.state = CommandBufferState::RECORDING;
}

void command_buffer_end(CommandBuffer cmd) {
	VkResult result = vkEndCommandBuffer(cmd.command_buffer);
	TK_ASSERT(result);
	cmd.state = CommandBufferState::RECORDING;
}

}  // namespace toki::renderer
