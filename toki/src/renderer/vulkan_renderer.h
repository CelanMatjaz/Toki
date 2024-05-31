#pragma once

#include "toki/renderer/renderer.h"

namespace Toki {

class VulkanRenderer : public Renderer {
private:
    virtual void init() override;
    virtual void shutdown() override;

    virtual void bindWindow(Ref<Window> window) override;

    virtual void beginFrame(const FrameData& data) override;
    virtual void endFrame(const FrameData& data) override;
    virtual void present(const FrameData& data) override;

public:
    virtual void setViewport(const Region2D& region) override;
    virtual void resetViewport() override;
    virtual void setScissor(const Region2D& region) override;
    virtual void resetScissor() override;
    virtual void setLineWidth(float width) override;

    virtual Handle createFramebuffer(const FramebufferConfig& config) override;
    virtual void destroyFramebuffer(Handle handle) override;
    virtual void bindFramebuffer(Handle handle) override;
    virtual void unbindFramebuffer() override;

    virtual Handle createShader(const ShaderConfig& config) override;
    virtual void destroyShader(Handle handle) override;
    virtual void bindShader(Handle handle) override;

    virtual Handle createBuffer(const BufferConfig& config) override;
    virtual Handle createBuffer(BufferType bufferType, uint32_t size) override;
    virtual void destroyBuffer(Handle handle) override;
    virtual void setBufferData(Handle handle, uint32_t size, uint32_t offset, void* data) override;
    virtual void bindIndexBuffer(Handle handle, uint32_t offset = 0) override;
    virtual void bindVertexBuffers(const std::vector<VertexBufferBinding>& bufferBindings) override;

    virtual Handle createTexture(ColorFormat format, std::filesystem::path path) override;
    virtual Handle createTexture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers = 1) override;
    virtual Handle createTexture(const TextureConfig& config) override;
    virtual void destroyTexture(Handle handle) override;

    virtual Handle createSampler() override;
    virtual void destroySampler(Handle handle) override;

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
    bool m_wasWindowResized = false;
};

}  // namespace Toki
