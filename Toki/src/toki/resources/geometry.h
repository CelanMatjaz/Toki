#pragma once

#include "core/core.h"
#include "filesystem"
#include "functional"
#include "renderer/buffer.h"

namespace Toki {

    struct Vertex {
        glm::vec3 position{};
        glm::vec2 uv{};
        glm::vec3 normal{};

        bool operator==(const Vertex& other) const {
            return position == other.position /* && color == other.color */ && normal == other.normal && uv == other.uv;
        }

        bool operator!=(const Vertex& other) const {
            return position != other.position /* || color != other.color */ || normal != other.normal || uv != other.uv;
        }
    };

    struct GeometryData {
        std::vector<Vertex> vertexData;
        std::vector<uint32_t> indexData;
    };

    class Geometry {
    public:
        Geometry();
        ~Geometry();
        static Ref<Geometry> create();

        static void mappingFunction();

        void loadFromObj(std::filesystem::path path);

        Ref<VertexBuffer> getVertexBuffer() { return vertexBuffer; }
        Ref<IndexBuffer> getIndexBuffer() { return indexBuffer; }

    private:
        Ref<VertexBuffer> vertexBuffer;
        Ref<IndexBuffer> indexBuffer;
    };

}