#pragma once

#include "../core/types.h"

namespace toki {

struct Vec2 {
    f32 x, y;

    Vec2 add(const Vec2& other) {
        return { x + other.x, y + other.y };
    }

    Vec2 add(f32 value) {
        return { x + value, y + value };
    }
};

struct Vec3 {
    f32 x, y, z;
};

struct Vec4 {
    f32 x, y, z, w;
};

struct Rect2d {
    Vec2 pos;
    Vec2 size;
}

}  // namespace toki
