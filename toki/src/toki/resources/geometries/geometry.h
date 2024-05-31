#pragma once

#include <cstdint>

#include "toki/core/containers.h"
#include "toki/core/core.h"
#include "toki/resources/resource.h"

namespace Toki {

struct VertexPoint {
    Point3D position;
};

struct VertexLine {
    Point3D position;
};

struct VertexFill {
    Point3D position;
    Point3D normal;
    Point2D uv;
};

enum class GeometryType {
    Points,
    Lines,
    Fill
};

struct GeometryConfig {
    GeometryType type;
};

class Geometry {
public:
    static Ref<Geometry> create(const GeometryConfig& config);

    void loadFromFile(const std::filesystem::path& path);
    void setVertexData(uint32_t size, void* data);
    void setIndexData(uint32_t size, void* data);

private:
    Geometry() = delete;
    Geometry(const GeometryConfig& config);

    GeometryConfig m_config;

    uint16_t m_vertexBufferIndex = 0;
    uint16_t m_indexBufferIndex = 0;

    uint32_t m_vertexBufferOffset = 0;
    uint32_t m_vertexCount = 0;

    uint32_t m_indexBufferOffset = 0;
    uint32_t m_indexCount = 0;

    glm::mat4 m_modelMatrix{ 1.0f };
};

}  // namespace Toki
