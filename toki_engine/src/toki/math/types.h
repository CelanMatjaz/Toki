#pragma once

#include "math/vector.h"

namespace toki {

struct rect2d {
    vector2<u32> pos;
    vector2<u32> size;
};

using extent2D = vector2<u32>;
using offset2D = vector2<u32>;

using extent3D = vector2<u32>;
using offset3D = vector2<u32>;

}  // namespace toki
