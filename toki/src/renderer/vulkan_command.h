#pragma once

#include "renderer_state.h"
#include "toki/renderer/command.h"

namespace Toki {

class VulkanCommand : public Command {
public:
    VulkanCommand(VkCommandBuffer commandBuffer);

public:
    virtual void setViewport(const Region2D& region) override;
    virtual void resetViewport() override;
    virtual void setScissor(const Region2D& region) override;
    virtual void resetScissor() override;
    virtual void setLineWidth(float width) override;

    virtual void bindFramebuffer(Handle handle) override;
    virtual void unbindFramebuffer() override;

    virtual void bindShader(Handle handle) override;

    virtual void bindIndexBuffer(Handle handle, uint32_t offset = 0) override;
    virtual void bindVertexBuffers(const std::vector<VertexBufferBinding>& bufferBindings) override;

    virtual void uploadGeometry(Ref<Geometry> geometry) override;

    virtual void setColorClear(const Color& color) override;
    virtual void setDepthClear(float clear) override;
    virtual void setStencilClear(uint32_t clear) override;

    virtual void setTexture(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) override;
    virtual void setSampler(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) override;
    virtual void setUniformBuffer(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) override;
    virtual void setTextureWithSampler(
        Handle textureHandle, Handle samplerHandle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) override;
    virtual void bindUniforms(uint32_t firstSet, uint32_t setCount) override;

    virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
    virtual void drawIndexed(
        uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

private:
    VkCommandBuffer m_commandBuffer;
};

}  // namespace Toki
