#pragma once

#include <toki/core/types.h>

namespace toki {

template <typename T>
struct Vec2 {
	T x, y;
};

using f32Vec2 = Vec2<f32>;
using i32Vec2 = Vec2<i32>;
using Vec2u32 = Vec2<u32>;

}  // namespace toki
