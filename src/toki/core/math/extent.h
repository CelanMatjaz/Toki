#pragma once

#include <toki/core/math/vector2.h>
#include <toki/core/math/vector3.h>

namespace toki {

template <typename T1, typename T2 = T1>
struct _extent2d {
	_vector2<T1> pos;
	_vector2<T2> size;
};

using Extent2D = _extent2d<f32>;

struct Extent3D {
	Vector3 pos;
	Vector3 size;
};

}  // namespace toki
