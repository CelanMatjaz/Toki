#pragma once

#include <toki/core/core.h>

namespace toki {

class Camera {
public:
	Camera() {};

	void set_projection(const Matrix4& projection);
	void set_view(const Matrix4& view);

	void set_position(const Vector3& position);
	void set_rotation(const Vector3& rotation);

	const Matrix4& get_view();
	const Matrix4& get_projection() const;
	const Matrix4& get_view_projection() const;

	Vector3 forward() const;
	Vector3 right() const;
	Vector3 up() const;

	void recalculate_if_dirty();

private:
	Matrix4 m_view{};
	Matrix4 m_projection{};
	Vector3 m_position{};
	Vector3 m_rotation{};
	b8 m_dirty = true;
};

}  // namespace toki
