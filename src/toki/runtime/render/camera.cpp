#include "toki/runtime/render/camera.h"

namespace toki {

void Camera::set_projection(const Matrix4& projection) {
	m_projection = projection;
	m_projection[5] *= -1;
	m_dirty = true;
}

void Camera::set_view(const Matrix4& view) {
	m_view	= view;
	m_dirty = true;
}

const Matrix4& Camera::get_view() {
	recalculate_if_dirty();
	return m_view;
}

const Matrix4& Camera::get_projection() const {
	return m_projection;
}

void Camera::set_position(const Vector3& position) {
	m_position = position;
	m_dirty	   = true;
}

void Camera::set_rotation(const Vector3& rotation) {
	m_rotation = rotation;
	m_dirty	   = true;
}

Vector3 Camera::forward() const {
	f32 yaw	  = radians(m_rotation.y);
	f32 pitch = radians(m_rotation.x);

	Vector3 f;
	f.x = cos(yaw) * cos(pitch);
	f.y = sin(pitch);
	f.z = sin(yaw) * cos(pitch);
	return f.normalize();
}

Vector3 Camera::right() const {
	return Vector3(0.0f, 1.0f, 0.0f).cross(forward()).normalize();
}

Vector3 Camera::up() const {
	return forward().cross(right());
}

void Camera::recalculate_if_dirty() {
	if (!m_dirty) {
		return;
	}

	Vector3 direction;
	direction.x		= cos(radians(m_rotation.y)) * cos(radians(m_rotation.x));
	direction.y		= sin(radians(m_rotation.x));
	direction.z		= sin(radians(m_rotation.y)) * cos(radians(m_rotation.x));
	Vector3 forward = direction.normalize();
	m_view			= look_at(m_position, m_position + forward, { 0.0f, 1.0f, 0.0f });
}

}  // namespace toki
