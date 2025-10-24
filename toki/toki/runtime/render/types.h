#pragma once

#include <toki/core/math/vector2.h>
#include <toki/core/math/vector3.h>

namespace toki {

struct Vertex {
	Vector3 position;
	Vector3 normals;
	Vector2 uv;

	b8 operator<=>(const Vertex&) const = default;
};

}  // namespace toki
