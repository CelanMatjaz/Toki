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

}  // namespace toki
