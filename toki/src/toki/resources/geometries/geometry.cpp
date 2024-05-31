#include "geometry.h"

namespace Toki {

Ref<Geometry> Geometry::create(const GeometryConfig& config) {
    std::unreachable();
    return nullptr;
}

Geometry::Geometry(const GeometryConfig& config) : m_config(config) {}

void Geometry::loadFromFile(const std::filesystem::path& path) {}

void Geometry::setVertexData(uint32_t size, void* data) {}

void Geometry::setIndexData(uint32_t size, void* data) {}

}  // namespace Toki
