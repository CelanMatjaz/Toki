#pragma once

#include "toki/core/containers.h"
#include "toki/core/core.h"
#include "toki/core/frame_data.h"
#include "toki/core/window.h"
#include "toki/renderer/command.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/configs.h"

namespace Toki {

class Application;

class Renderer {
    friend Application;

public:
    virtual void submit(SubmitFunction submitFn) = 0;

private:
    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual void bindWindow(Ref<Window> window) = 0;

    virtual void beginFrame(const FrameData& data) = 0;
    virtual void endFrame(const FrameData& data) = 0;
    virtual void present(const FrameData& data) = 0;

public:
    virtual Handle createFramebuffer(const FramebufferConfig& config) = 0;
    virtual void destroyFramebuffer(Handle handle) = 0;

    virtual Handle createShader(const ShaderConfig& config) = 0;
    virtual void destroyShader(Handle handle) = 0;

    virtual Handle createBuffer(const BufferConfig& config) = 0;
    virtual Handle createBuffer(BufferType bufferType, uint32_t size) = 0;
    virtual void destroyBuffer(Handle handle) = 0;
    virtual void setBufferData(Handle handle, uint32_t size, uint32_t offset, void* data) = 0;

    virtual Handle createTexture(ColorFormat format, std::filesystem::path path) = 0;
    virtual Handle createTexture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers = 1) = 0;
    virtual Handle createTexture(const TextureConfig& config) = 0;
    // TODO: add set data for textures
    virtual void destroyTexture(Handle handle) = 0;

    virtual Handle createSampler() = 0;
    virtual void destroySampler(Handle handle) = 0;

public:
    virtual ~Renderer() = default;
};

class Renderer3D {
public:
    static void flush();

private:
};

class Renderer2D {
public:
    static void flush();

    static void drawQuad(const Rect2D& quad, const Color& color);
    static void drawQuad(const Rect2D& quad, Handle texture);

    static void drawQuadClipped(const Rect2D& quad, const Color& color, const Rect2D& clip);
    static void drawQuadClipped(const Rect2D& quad, Handle texture, const Rect2D& clip);

private:
};

class RendererDebug {
public:
private:
};

static Scope<Renderer> renderer;

}  // namespace Toki
