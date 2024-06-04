#pragma once

#include "toki/renderer/renderer.h"

namespace Toki {

class VulkanRenderer : public Renderer {
private:
    virtual void init() override;
    virtual void shutdown() override;

    virtual void bindWindow(Ref<Window> window) override;

    virtual bool beginFrame(const FrameData& data) override;
    virtual void endFrame(const FrameData& data) override;
    virtual void present(const FrameData& data) override;

    virtual void submit(SubmitFunction submitFn) override;

public:
    virtual Handle createFramebuffer(const FramebufferConfig& config) override;
    virtual void destroyFramebuffer(Handle handle) override;

    virtual Handle createShader(const ShaderConfig& config) override;
    virtual void destroyShader(Handle handle) override;

    virtual Handle createBuffer(const BufferConfig& config) override;
    virtual Handle createBuffer(BufferType bufferType, uint32_t size) override;
    virtual void destroyBuffer(Handle handle) override;
    virtual void setBufferData(Handle handle, uint32_t size, uint32_t offset, void* data) override;

    virtual Handle createTexture(ColorFormat format, std::filesystem::path path) override;
    virtual Handle createTexture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers = 1) override;
    virtual Handle createTexture(const TextureConfig& config) override;
    // TODO: add set data for textures
    virtual void destroyTexture(Handle handle) override;

    virtual Handle createSampler() override;
    virtual void destroySampler(Handle handle) override;

private:
    bool m_wasWindowResized = false;
};

}  // namespace Toki
