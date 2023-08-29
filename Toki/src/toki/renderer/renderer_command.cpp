#include "tkpch.h"
#include "renderer_command.h"
#include "vulkan/vulkan.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/vulkan_buffer.h"
#include "platform/vulkan/vulkan_shader.h"
#include "platform/vulkan/vulkan_texture.h"
#include "unordered_map"

namespace Toki {

    void RendererCommand::setViewport(const glm::vec2& position, const glm::vec2& extent) {
        VkViewport viewport{};
        viewport.x = position.x;
        viewport.y = position.y;
        viewport.width = extent.x;
        viewport.height = extent.y;
        vkCmdSetViewport(VulkanRenderer::commandBuffer(), 0, 1, &viewport);
    }

    void RendererCommand::setScissor(const glm::ivec2& position, const glm::ivec2& extent) {
        VkRect2D scissor{};
        scissor.offset = { position.x, position.y };
        scissor.extent = { (uint32_t) extent.x, (uint32_t) extent.y };
        vkCmdSetScissor(VulkanRenderer::commandBuffer(), 0, 1, &scissor);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer) {
        vertexBuffer->bind();
        vkCmdDraw(VulkanRenderer::commandBuffer(), ((VulkanVertexBuffer*) vertexBuffer.get())->getBinding(), 1, 0, 0);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer, uint32_t verteciesCount) {
        vertexBuffer->bind();
        vkCmdDraw(VulkanRenderer::commandBuffer(), verteciesCount, 1, 0, 0);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer) {
        vertexBuffer->bind();
        indexBuffer->bind();
        vkCmdDrawIndexed(VulkanRenderer::commandBuffer(), indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }

    void RendererCommand::draw(Ref<Geometry> geometry) {
        RendererCommand::draw(geometry->getVertexBuffer(), geometry->getIndexBuffer());
    }

    void RendererCommand::drawInstanced(const std::vector<Ref<VertexBuffer>>& vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount) {
        std::unordered_map<uint32_t, std::vector<VkBuffer>> bufferMap;

        for (const auto& vertexBuffer : vertexBuffers) {
            uint32_t binding = vertexBuffer->getBinding();
            if (!bufferMap.contains(binding))
                bufferMap[binding] = {};
            bufferMap[binding].emplace_back(((VulkanVertexBuffer*) vertexBuffer.get())->getBuffer());
        }

        VkCommandBuffer commandBuffer = VulkanRenderer::commandBuffer();

        VkDeviceSize offsets[] = { 0 };

        for (const auto& [binding, buffers] : bufferMap) {
            vkCmdBindVertexBuffers(commandBuffer, binding, 1, buffers.data(), offsets);
        }

        indexBuffer->bind();

        vkCmdDrawIndexed(commandBuffer, indexBuffer->getIndexCount(), instanceCount, 0, 0, 0);
    }

    void RendererCommand::drawInstanced(Ref<Geometry> geometry, Ref<VertexBuffer> instanceBuffer, uint32_t instanceCount) {
        drawInstanced({ geometry->getVertexBuffer(), instanceBuffer }, geometry->getIndexBuffer(), instanceCount);
    }

    void RendererCommand::setConstant(Ref<Shader> shader, uint32_t dataSize, const void* data) {
        VulkanShader* vulkanShader = (VulkanShader*) shader.get();
        vkCmdPushConstants(VulkanRenderer::commandBuffer(), ((VulkanShader*) shader.get())->getPipelineLayout(), vulkanShader->getContantStageFlags(), 0, dataSize, data);
    }

    void RendererCommand::setUniform(Ref<Shader> shader, Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t index, uint32_t set) {
        VulkanShader* vulkanShader = (VulkanShader*) shader.get();
        VulkanUniformBuffer* vulkanUniform = (VulkanUniformBuffer*) uniformBuffer.get();
        vulkanShader->setUniform(uniformBuffer, binding, set, index);
        VkDescriptorSet descriptorSet = vulkanShader->getSet(set);
        vkCmdBindDescriptorSets(VulkanRenderer::commandBuffer(), vulkanShader->getPipelineBindPoint(), vulkanShader->getPipelineLayout(), set, 1, &descriptorSet, 0, nullptr);
    }

    void RendererCommand::setTexture(Ref<Shader> shader, Ref<Texture> texture, uint32_t binding, uint32_t index, uint32_t set) {
        VulkanShader* vulkanShader = (VulkanShader*) shader.get();
        VulkanTexture* vulkanTexture = (VulkanTexture*) texture.get();
        vulkanShader->setTexture(texture, binding, set, index);
        VkDescriptorSet descriptorSet = vulkanShader->getSet(set);
        vkCmdBindDescriptorSets(VulkanRenderer::commandBuffer(), vulkanShader->getPipelineBindPoint(), vulkanShader->getPipelineLayout(), set, 1, &descriptorSet, 0, nullptr);
    }

}