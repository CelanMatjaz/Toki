#include "toki/runtime/render/camera.h"

namespace toki {

void Camera::set_projection(const Matrix4& projection) {
	m_projection = projection;
	m_projection[5] *= -1;
	m_dirty = true;
}

void Camera::set_view(const Matrix4& view) {
	m_view = view;
	m_dirty = true;
}

const Matrix4& Camera::get_view() {
	return m_view;
}

const Matrix4& Camera::get_projection() {
	return m_projection;
}

void Camera::set_position(const Vector3& position) {
	m_position = position;
}

void Camera::set_rotation(const Vector3& rotation) {
	Vector3 direction;
	direction.x = cos(convert_angle_to(rotation.y)) * cos(convert_angle_to(rotation.x));
	direction.y = sin(convert_angle_to(rotation.x));
	direction.z = sin(convert_angle_to(rotation.y)) * cos(convert_angle_to(rotation.x));
	m_forward = direction.normalize();
	m_view = look_at(m_position, m_position + m_forward, { 0.0f, 1.0f, 0.0f });
}

Vector3 Camera::forward() const {
	return Vector3(-m_view[2 * 4 + 0], -m_view[2 * 4 + 1], -m_view[2 * 4 + 2]).normalize();
}

Vector3 Camera::right() const {
	return Vector3(0.0f, 1.0f, 0.0f).cross(forward()).normalize();
}

Vector3 Camera::up() const {
	return forward().cross(right());
}

}  // namespace toki
