#pragma once

#include "core/base.h"

namespace toki {

using Vec2 = glm::vec<2, u32>;
using Vec3 = glm::vec<3, u32>;
using Vec4 = glm::vec<4, u32>;

struct Rect2D {
    glm::vec<2, i32> pos;
    Vec2 size;
};

}  // namespace toki
