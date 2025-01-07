#pragma once

#include "core/base.h"

namespace toki {

template <typename T>
struct Vector2 {
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
struct Vector3 {
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
struct Vector4 {
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

using Vec4i = Vector4<u32>;
using Vec4f = Vector4<f32>;

using Vec3i = Vector3<u32>;
using Vec3f = Vector3<f32>;

using Vec2i = Vector2<u32>;
using Vec2f = Vector2<f32>;

}  // namespace toki
