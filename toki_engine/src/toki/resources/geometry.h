#pragma once

#include "memory/allocators/basic_allocator.h"
#include "resources/resources.h"

namespace toki {

class Geometry {
public:
    Geometry();
    ~Geometry();

    void load_data();

private:
    BasicRef<u32> m_indexData;
    BasicRef<byte> m_vertexData;

    glm::vec3 m_origin{};
};

}  // namespace toki
