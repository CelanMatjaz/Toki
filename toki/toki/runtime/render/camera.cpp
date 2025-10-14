#include "toki/runtime/render/camera.h"

namespace toki {

void Camera::set_projection(const Matrix4& projection) {
	m_projection = projection;
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

const Matrix4& Camera::get_view_projection() {
	if (m_dirty) {
		m_viewProjection = m_view * m_projection;
		m_dirty = false;
	}

	return m_viewProjection;
}

}  // namespace toki
