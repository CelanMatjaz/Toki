#pragma once

#include "../core/types.h"

namespace toki {

template <typename T = f32>
union Vec2 {
	struct {
		T x, y;
	};

	struct {
		T width, height;
	};
};

template <typename T = f32>
union Vec3 {
	struct {
		T x, y, z;
	};

	struct {
		T r, g, b;
	};
};

template <typename T = f32>
union Vec4 {
	struct {
		T x, y, z, w;
	};

	struct {
		T r, g, b, a;
	};
};

}  // namespace toki
