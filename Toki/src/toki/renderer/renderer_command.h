#pragma once

#include "glm/glm.hpp"
#include "buffer.h"
#include "shader.h"
#include "texture.h"
#include "resources/geometry.h"

namespace Toki {

    class VulkanContext;

    class RendererCommand {
        friend VulkanContext;

    public:
        static void setViewport(const glm::vec2& position, const glm::vec2& extent);
        static void setScissor(const glm::ivec2& position, const glm::ivec2& extent);
        static void setLineWidth(const float& width);

        static void draw(Ref<VertexBuffer> vertexBuffer);
        static void draw(Ref<VertexBuffer> vertexBuffer, uint32_t vertexCount);
        static void draw(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer);
        static void draw(Ref<Geometry> geometry);
        // static void draw(std::vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer);

        static void drawInstanced(const std::vector<Ref<VertexBuffer>>& vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount = 1, uint32_t firstInstance = 0);
        static void drawInstanced(Ref<Geometry> geometry, Ref<VertexBuffer> instanceBuffer, uint32_t instanceCount = 1);

        static void setConstant(Ref<Shader> shader, uint32_t dataSize, const void* data);
        static void setUniform(Ref<Shader> shader, Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t index = 0, uint32_t set = 0);
        static void setTexture(Ref<Shader> shader, Ref<Texture> texture, uint32_t binding, uint32_t index = 0, uint32_t set = 0);

        static void bindSets(Ref<Shader> shader, uint32_t firstSet = 0);
    };

}