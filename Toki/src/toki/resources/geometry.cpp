#include "tkpch.h"
#include "resources/file_loaders/model_loader.h"
#include "geometry.h"

namespace Toki {

    Geometry::Geometry() {

    }

    Geometry::~Geometry() {

    }

    Ref<Geometry> Geometry::create() {
        return createRef<Geometry>();
    }

    void Geometry::loadFromObj(std::filesystem::path path) {
        auto data = ModelLoader::loadFromObj(path);

        VertexBufferConfig vertexBufferConfig{};
        vertexBufferConfig.isStatic = true;
        vertexBufferConfig.size = data.vertexData.size() * sizeof(Vertex);
        vertexBufferConfig.binding = 0;
        vertexBuffer = VertexBuffer::create(vertexBufferConfig);
        vertexBuffer->setData(vertexBufferConfig.size, data.vertexData.data());

        IndexBufferConfig indexBufferConfig{};
        indexBufferConfig.isStatic = true;
        indexBufferConfig.size = data.indexData.size() * sizeof(uint32_t);
        indexBufferConfig.indexCount = data.indexData.size();
        indexBuffer = IndexBuffer::create(indexBufferConfig);
        indexBuffer->setData(indexBufferConfig.size, data.indexData.data());
    }

}