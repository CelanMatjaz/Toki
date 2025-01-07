#pragma once

#include "core/base.h"

namespace toki {

template <typename T>
struct vector2 {
    union {
        T x;
        T r;
        T width;
    };

    union {
        T y;
        T g;
        T height;
    };
};

template <typename T>
struct vector3 {
    union {
        T x;
        T r;
        T width;
    };

    union {
        T y;
        T g;
        T height;
    };

    union {
        T z;
        T b;
    };
};

template <typename T>
struct vector4 {
    union {
        T x;
        T r;
    };

    union {
        T y;
        T g;
    };

    union {
        T z;
        T b;
    };

    union {
        T w;
        T a;
    };
};

using Vec4i = vector4<u32>;
using Vec4f = vector4<f32>;

using Vec3i = vector3<u32>;
using Vec3f = vector3<f32>;

using Vec2i = vector2<u32>;
using Vec2f = vector2<f32>;

}  // namespace toki
