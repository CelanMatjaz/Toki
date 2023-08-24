#pragma once

#include "glm/glm.hpp"
#include "buffer.h"
#include "shader.h"

namespace Toki {

    class RendererCommand {
    public:
        static void setViewport(const glm::vec2& position, const glm::vec2& extent);
        static void setScissor(const glm::ivec2& position, const glm::ivec2& extent);

        static void draw(Ref<VertexBuffer> vertexBuffer);
        static void draw(Ref<VertexBuffer> vertexBuffer, uint32_t vertecies);
        static void draw(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer);
        // static void draw(std::vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer);

        static void drawInstanced(std::vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount = 1);

        static void setConstant(Ref<Shader> shader, ShaderStage stage, uint32_t dataSize, const void* data);
        static void setUniform(Ref<Shader> shader, Ref<UniformBuffer> uniformBuffer, ShaderStage stage, uint32_t binding, uint32_t set = 0);


    };

}