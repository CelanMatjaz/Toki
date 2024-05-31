#pragma once

#include <filesystem>

#include "toki/core/core.h"
#include "toki/resources/geometries/geometry.h"

namespace Toki {

class GeometryLoader {
public:
    static Ref<Geometry> loadFromObjFile(std::filesystem::path path);
};

}  // namespace Toki
