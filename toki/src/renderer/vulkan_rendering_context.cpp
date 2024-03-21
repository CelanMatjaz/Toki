#include "vulkan_rendering_context.h"

#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_shader.h"
#include "toki/core/assert.h"

namespace Toki {

VulkanRenderingContext::VulkanRenderingContext(VkCommandBuffer commandBuffer) : m_commandBuffer(commandBuffer) {}

void VulkanRenderingContext::bindVertexBuffers(std::vector<Ref<VertexBuffer>> vertexBuffers) const {
    std::vector<VkBuffer> buffers(vertexBuffers.size());
    std::vector<VkDeviceSize> offsets(vertexBuffers.size());

    for (uint32_t i = 0; i < vertexBuffers.size(); ++i) {
        buffers[i] = ((VulkanVertexBuffer*) vertexBuffers[i].get())->getHandle();
        offsets[i] = 0;
    }

    vkCmdBindVertexBuffers(m_commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
}

void VulkanRenderingContext::bindIndexBuffer(Ref<IndexBuffer> indexBuffer) const {
    vkCmdBindIndexBuffer(
        m_commandBuffer,
        ((VulkanIndexBuffer*) indexBuffer.get())->getHandle(),
        0,
        indexBuffer->getIndexSize() == IndexSize::INDEX_SIZE_32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
}

void VulkanRenderingContext::bindShader(Ref<Shader> shader) const {
    auto s = ((VulkanShader*) shader.get());
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s->getPipeline());
}

void VulkanRenderingContext::pushConstants(Ref<Shader> shader, uint32_t size, void* data, uint32_t offset) const {
    auto s = ((VulkanShader*) shader.get());
    vkCmdPushConstants(m_commandBuffer, s->getPipelineLayout(), s->getPushConstantStageFlags(), offset, size, data);
}

void VulkanRenderingContext::bindUniforms(Ref<Shader> shader, uint32_t firstSet, uint32_t setCount) const {
    auto s = ((VulkanShader*) shader.get());
    const std::vector<VkDescriptorSet>& sets = s->getDestriptorSets();
    TK_ASSERT(sets.size() >= setCount + firstSet, "Not enough sets bound {} >= {} + {}", sets.size(), setCount, firstSet);
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s->getPipelineLayout(), firstSet, setCount, sets.data(), 0, nullptr);
}

void VulkanRenderingContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderingContext::drawIndexed(
    uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VkCommandBuffer VulkanRenderingContext::getCommandBuffer() const {
    return m_commandBuffer;
}

}  // namespace Toki
