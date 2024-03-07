#include "vulkan_rendering_context.h"

#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_graphics_pipeline.h"

namespace Toki {

VulkanRenderingContext::VulkanRenderingContext(VkCommandBuffer commandBuffer) : m_commandBuffer(commandBuffer) {}

void VulkanRenderingContext::bindVertexBuffers(std::vector<Ref<VertexBuffer>> vertexBuffers) const {
    std::vector<VkBuffer> buffers = { ((VulkanVertexBuffer*) vertexBuffers[0].get())->getHandle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, buffers.data(), offsets);
}

void VulkanRenderingContext::bindIndexBuffer(Ref<IndexBuffer> indexBuffer) const {
    vkCmdBindIndexBuffer(m_commandBuffer, ((VulkanIndexBuffer*) indexBuffer.get())->getHandle(), 0, VK_INDEX_TYPE_UINT32);
}

void VulkanRenderingContext::bindShader(Ref<Shader> shader) const {
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ((VulkanGraphicsPipeline*) shader.get())->getPipeline());
}

void VulkanRenderingContext::pushConstants(Ref<Shader> shader, uint32_t size, void* data) const {
    vkCmdPushConstants(m_commandBuffer, ((VulkanGraphicsPipeline*) shader.get())->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
}

void VulkanRenderingContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VkCommandBuffer VulkanRenderingContext::getCommandBuffer() const {
    return m_commandBuffer;
}

}  // namespace Toki
