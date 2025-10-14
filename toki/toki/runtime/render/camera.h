#pragma once

#include <toki/core/core.h>

namespace toki {

class Camera {
public:
	Camera() {};

	void set_projection(const Matrix4& projection);
	void set_view(const Matrix4& view);

	const Matrix4& get_view();
	const Matrix4& get_projection();
	const Matrix4& get_view_projection();

private:
	Vector3 m_position{};
	b8 m_dirty = true;
	Matrix4 m_view{};
	Matrix4 m_projection{};
	Matrix4 m_viewProjection{};
};

}  // namespace toki
