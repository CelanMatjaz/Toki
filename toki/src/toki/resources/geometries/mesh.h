#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/resources/geometries/geometry.h"

namespace Toki {

struct MeshConfig {};

class Mesh {
public:
    static Ref<Mesh> create(const MeshConfig& config);

    void load();

private:
    MeshConfig m_config;

    std::vector<Ref<Geometry>> m_geometries;
};

}  // namespace Toki
