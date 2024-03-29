#pragma once

#include "toki/core/core.h"
#include "toki/renderer/buffer.h"
#include "toki/renderer/shader.h"

namespace Toki {

class RenderingContext {
public:
    RenderingContext() = default;

    virtual void bindVertexBuffers(std::vector<Ref<VertexBuffer>> vertexBuffers) const = 0;
    virtual void bindIndexBuffer(Ref<IndexBuffer> indexBuffer) const = 0;
    virtual void bindShader(Ref<Shader> shader) const = 0;

    virtual void pushConstants(Ref<Shader> shader, uint32_t size, void* data, uint32_t offset = 0) const = 0;
    virtual void bindUniforms(Ref<Shader> shader, uint32_t firstSet, uint32_t setCount) const = 0;

    virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const = 0;
    virtual void drawIndexed(
        uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const = 0;
};

}  // namespace Toki
