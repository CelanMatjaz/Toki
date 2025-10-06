#pragma once

#include <toki/core/types.h>

namespace toki {

template <typename T>
struct Vec2 {
	T x, y;
};

using Vec2f32 = Vec2<f32>;
using Vec2i32 = Vec2<i32>;
using Vec2u32 = Vec2<u32>;

template <typename T>
struct Vec3 {
	T x, y, z;
};

using Vec3f32 = Vec3<f32>;
using Vec3i32 = Vec3<i32>;
using Vec3u32 = Vec3<u32>;

}  // namespace toki
