#pragma once

#include "filesystem"
#include "resources/geometry.h"

namespace Toki {

    class ModelLoader {
    public:

        static GeometryData loadFromObj(std::filesystem::path path);


    };

}