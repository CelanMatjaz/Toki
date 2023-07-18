#pragma once

#include "tkpch.h"
#include "toki/core/model.h"

namespace Toki {
    struct GeometryData {
        std::vector<Vertex> vertexData;
        std::vector<uint32_t> indexData;
    };

    GeometryData loadObj(const char* objFile);
}