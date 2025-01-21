#include "vulkan_commands.h"

namespace toki {

namespace vulkan_commands {

void submit_single_use_command_buffer(RendererContext* ctx, std::function<void(VkCommandBuffer cmd)> fn) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = ctx->extra_command_pools.front();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer command_buffer;
    VK_CHECK(vkAllocateCommandBuffers(ctx->device, &command_buffer_allocate_info, &command_buffer), "Could not allocate single use command buffer");

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info), "Could not begin recording single use command buffer");

    fn(command_buffer);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    VK_CHECK(vkQueueSubmit(ctx->queues.transfer, 1, &submit_info, VK_NULL_HANDLE), "");
    VK_CHECK(vkQueueWaitIdle(ctx->queues.transfer), "");
    vkFreeCommandBuffers(ctx->device, ctx->extra_command_pools.front(), 1, &command_buffer);
}

}  // namespace vulkan_commands

}  // namespace toki
