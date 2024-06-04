#include "command_pool.h"

#include "renderer/renderer_state.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"
#include "toki/core/logging.h"

namespace Toki {

CommandPool::CommandPool(uint32_t initialCommandBufferCount) {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = context.physicalDeviceData.graphicsFamilyIndex.value();

    TK_ASSERT_VK_RESULT(
        vkCreateCommandPool(context.device, &commandPoolCreateInfo, context.allocationCallbacks, &m_commandPool), "Could not create command pool");

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = m_commandPool;
    commandBufferAllocateInfo.commandBufferCount = initialCommandBufferCount;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    m_allocatedCommandBuffers.resize(initialCommandBufferCount, VK_NULL_HANDLE);
    TK_ASSERT_VK_RESULT(
        vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, m_allocatedCommandBuffers.data()), "Could not allocate command buffers");
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(context.device, m_commandPool, context.allocationCallbacks);
}

void beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse);

VkCommandBuffer CommandPool::getNewCommandBuffer() {
    if (m_inUseCommandBufferCount < m_allocatedCommandBuffers.size()) {
        VkCommandBuffer commandBuffer = m_allocatedCommandBuffers[m_inUseCommandBufferCount];
        beginRecordingCommandBuffer(commandBuffer, true, false, false);
        ++m_inUseCommandBufferCount;
        return commandBuffer;
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = m_commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer newCommandBuffer;
    TK_ASSERT_VK_RESULT(
        vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, &newCommandBuffer), "Could not allocate command new command buffer");
    m_allocatedCommandBuffers.emplace_back(newCommandBuffer);
    beginRecordingCommandBuffer(newCommandBuffer, true, false, false);

    LOG_INFO("Created new command buffer for thread {}", std::this_thread::get_id());

    ++m_inUseCommandBufferCount;
    return newCommandBuffer;
}

void CommandPool::endCommandBuffers() {
    for (uint32_t i = 0; i < m_inUseCommandBufferCount; ++i) {
        TK_ASSERT_VK_RESULT(vkEndCommandBuffer(m_allocatedCommandBuffers[i]), "Could not end recording command buffer");
    }
}

void CommandPool::resetCommandBuffers() {
    for (uint32_t i = 0; i < m_inUseCommandBufferCount; ++i) {
        vkResetCommandBuffer(m_allocatedCommandBuffers[i], 0);
    }
    m_inUseCommandBufferCount = 0;
}

const std::vector<VkCommandBuffer> CommandPool::getSubmittableCommandBuffers() const {
    if (m_inUseCommandBufferCount == m_allocatedCommandBuffers.size()) {
        return m_allocatedCommandBuffers;
    }

    std::vector<VkCommandBuffer> commandBuffers;

    for (uint32_t i = 0; i < m_inUseCommandBufferCount; ++i) {
        commandBuffers.emplace_back(m_allocatedCommandBuffers[i]);
    }

    return commandBuffers;
}

void CommandPool::submitSingleUseCommands(VkQueue queue, std::function<void(VkCommandBuffer cmd)> fn) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = s_extraCommandPools[0];
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer;
    TK_ASSERT_VK_RESULT(
        vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, &commandBuffer), "Could not allocate single use command buffer");

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Could not begin recording single use command buffer");

    fn(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    TK_ASSERT_VK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE), "");
    TK_ASSERT_VK_RESULT(vkQueueWaitIdle(queue), "");
    vkFreeCommandBuffers(context.device, s_extraCommandPools[0], 1, &commandBuffer);
}

void beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // Flag explanations
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferUsageFlagBits.html
    if (isSingleUse) {
        commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    if (isRenderPassContinue) {
        commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    if (isSimultaneousUse) {
        commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Could not begin command buffer");
}

}  // namespace Toki
