#pragma once

#include "../core/types.h"

namespace toki {

template <typename T, u32 N>
struct Data {
    T data[N];
};

template <typename T, u32 N>
struct Vector;

template <typename T, u32 R, u32 C>
struct Matrix;

template <typename T>
struct Vec2 {
    T x;
    T y;
};

template <typename T>
struct Vec3 {
    union {
        T x, r;
    };
    union {
        T y, g;
    };
    union {
        T z, b;
    };
};

template <typename T>
struct Vec4 {
    union {
        T x, r;
    };
    union {
        T y, g;
    };
    union {
        T z, b;
    };
    union {
        T w, a;
    };

    union {
        Data<T, 4> data;
    };
};

struct Rect2D {
    Rect2D(Vec2<i32> pos, Vec2<u32> size): pos(pos), size(size) {}

    Vec2<i32> pos;
    Vec2<u32> size;
};

}  // namespace toki
