#include "command_pool.h"

#include "renderer/renderer_state.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"

namespace Toki {

CommandPool::CommandPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = context.physicalDeviceData.graphicsFamilyIndex.value();

    TK_ASSERT_VK_RESULT(
        vkCreateCommandPool(context.device, &commandPoolCreateInfo, context.allocationCallbacks, &m_commandPool), "Could not create command pool");
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(context.device, m_commandPool, context.allocationCallbacks);
}

CommandBuffer& CommandPool::allocateCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = m_commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer commandBuffer;
    TK_ASSERT_VK_RESULT(vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, &commandBuffer), "Could not allocate command buffers");

    m_allocatedCommandBuffers.emplace_back(commandBuffer);

    return m_allocatedCommandBuffers.back();
}

CommandBuffer& CommandPool::getCommandBuffer() {
    return m_allocatedCommandBuffers.front();
}

void CommandPool::beginCommandBuffers() {
    for (auto& commandBuffer : m_allocatedCommandBuffers) {
        if (commandBuffer.m_state != CommandBufferState::Initial) {
            continue;
        }
        commandBuffer.beginRecording(false, false, false);
    }
}

void CommandPool::endCommandBuffers() {
    for (auto& commandBuffer : m_allocatedCommandBuffers) {
        commandBuffer.endRecording();
        // if (commandBuffer.m_didRecordCommands) {}
    }
}

void CommandPool::resetCommandBuffers() {
    m_submittableCommandBuffers.clear();
    for (auto& commandBuffer : m_allocatedCommandBuffers) {
        commandBuffer.reset();
    }
}

void CommandPool::setSubmittableCommandBuffers() {
    for (const auto& commandBuffer : m_allocatedCommandBuffers) {
        if (commandBuffer.m_state == CommandBufferState::Executable && commandBuffer.m_didRecordCommands) {
            m_submittableCommandBuffers.emplace_back(commandBuffer);
        }
    }
}

const std::vector<VkCommandBuffer>& CommandPool::getSubmittableCommandBuffers() const {
    return m_submittableCommandBuffers;
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

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    TK_ASSERT_VK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE), "");
    TK_ASSERT_VK_RESULT(vkQueueWaitIdle(queue), "");
    vkFreeCommandBuffers(context.device, s_extraCommandPools[0], 1, &commandBuffer);
}

void CommandBuffer::beginRecording(bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse) {
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

    TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo), "Could not begin command buffer");
    m_state = CommandBufferState::Recording;
}

void CommandBuffer::endRecording() {
    TK_ASSERT_VK_RESULT(vkEndCommandBuffer(m_commandBuffer), "Could not end command buffer");
    m_state = CommandBufferState::Executable;
}

void CommandBuffer::reset() {
    vkResetCommandBuffer(m_commandBuffer, 0);
    m_state = CommandBufferState::Initial;
}

}  // namespace Toki
