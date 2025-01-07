#pragma once

#include "math/vector.h"

namespace toki {

struct Rect2D {
    Vector2<u32> pos;
    Vector2<u32> size;
};

using Extent2D = Vector2<u32>;
using Offset2D = Vector2<u32>;

using Extent3D = Vector2<u32>;
using Offset3D = Vector2<u32>;

}  // namespace toki
