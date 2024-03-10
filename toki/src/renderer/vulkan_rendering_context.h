#pragma once

#include <vulkan/vulkan.h>

#include "toki/renderer/rendering_context.h"

namespace Toki {

class VulkanRenderingContext : public RenderingContext {
public:
    VulkanRenderingContext(VkCommandBuffer commandBuffer);

    virtual void bindVertexBuffers(std::vector<Ref<VertexBuffer>> vertexBuffers) const override;
    virtual void bindIndexBuffer(Ref<IndexBuffer> indexBuffer) const override;
    virtual void bindShader(Ref<Shader> shader) const override;

    virtual void pushConstants(Ref<Shader> shader, uint32_t size, void* data) const override;
    virtual void bindUniforms(Ref<Shader> shader, uint32_t firstSet, uint32_t setCount) const override;

    virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const override;

    VkCommandBuffer getCommandBuffer() const;

private:
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    bool m_shouldUpdateDescriptorSets = false;
};

}  // namespace Toki
