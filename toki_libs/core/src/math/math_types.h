#pragma once

#include "../core/types.h"

namespace toki {

template <typename T, int N>
struct Data {
    T data[N]{};
};

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
    union {
        f32 x, y, z, w;
    };
    union {
        f32 r, g, b, a;
    };
};

struct Rect2D {
    Vec2 pos;
    Vec2 size;
};

}  // namespace toki
