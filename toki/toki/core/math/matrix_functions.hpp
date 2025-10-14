#pragma once

#include "toki/core/math/matrix4.h"
#include "toki/core/math/vector3.h"

namespace toki {

constexpr Matrix4 look_at(Vector3 position, Vector3 direction, Vector3 up) {
	Vector3 f = (direction - position).normalize();
	Vector3 r = f.cross(up).normalize();
	Vector3 u = r.cross(f);

	return Matrix4(
		r.x,
		u.x,
		-f.x,
		0.0f,
		r.y,
		u.y,
		-f.y,
		0.0f,
		r.z,
		u.z,
		-f.z,
		0.0f,
		-r.dot(position),
		-u.dot(position),
		f.dot(position),
		1.0f);
}

constexpr Matrix4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	f32 rl = right - left;
	f32 tb = top - bottom;
	f32 fn = far - near;

	return Matrix4(
		2.0f / rl,
		0.0f,
		0.0f,
		-(right + left) / rl,
		0.0f,
		2.0f / tb,
		0.0f,
		-(top + bottom) / tb,
		0.0f,
		0.0f,
		-2.0f / fn,
		-(far + near) / fn,
		0.0f,
		0.0f,
		0.0f,
		1.0f);
}


}  // namespace toki
