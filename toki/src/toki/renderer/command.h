#pragma once

#include <functional>

#include "toki/core/containers.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/configs.h"
#include "toki/resources/geometries/geometry.h"

namespace Toki {

class Command;

using SubmitFunction = std::function<void(Command&)>;

class Command {
public:
    virtual void setViewport(const Region2D& region) = 0;
    virtual void resetViewport() = 0;
    virtual void setScissor(const Region2D& region) = 0;
    virtual void resetScissor() = 0;
    virtual void setLineWidth(float width) = 0;

    virtual void bindFramebuffer(Handle handle) = 0;
    virtual void unbindFramebuffer() = 0;

    virtual void bindShader(Handle handle) = 0;

    virtual void bindIndexBuffer(Handle handle, uint32_t offset = 0) = 0;
    virtual void bindVertexBuffers(const std::vector<VertexBufferBinding>& bufferBindings) = 0;

    virtual void uploadGeometry(Ref<Geometry> geometry) = 0;

    virtual void setColorClear(const Color& color) = 0;
    virtual void setDepthClear(float clear) = 0;
    virtual void setStencilClear(uint32_t clear) = 0;

    virtual void setTexture(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) = 0;
    virtual void setSampler(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) = 0;
    virtual void setTextureWithSampler(Handle textureHandle, Handle samplerHandle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) = 0;
    virtual void setUniformBuffer(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex = 0) = 0;
    virtual void bindUniforms(uint32_t firstSet, uint32_t setCount) = 0;

    virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;
};

}  // namespace Toki
