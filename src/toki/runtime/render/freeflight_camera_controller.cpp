#include <toki/runtime/render/freeflight_camera_controller.h>

namespace toki {

Camera& FreeFlightCameraController::camera() {
	return m_camera;
}

void FreeFlightCameraController::on_update(f32 delta_time, const Window* window) {
	m_camera.recalculate_if_dirty();

	f32 forward_direction = 0;
	if (window->is_key_down(toki::Key::W)) {
		forward_direction += +1;
	}
	if (window->is_key_down(toki::Key::S)) {
		forward_direction += -1;
	}
	m_position += m_camera.forward() * forward_direction * m_speed * delta_time;

	f32 strafe_direction = 0;
	if (window->is_key_down(toki::Key::A)) {
		strafe_direction += +1;
	}
	if (window->is_key_down(toki::Key::D)) {
		strafe_direction += -1;
	}
	m_position += m_camera.right() * strafe_direction * m_speed * delta_time;

	f32 vertical_direction = 0;
	if (window->is_key_down(toki::Key::LEFT_SHIFT)) {
		vertical_direction += +1;
	}
	if (window->is_key_down(toki::Key::LEFT_CONTROL)) {
		vertical_direction += -1;
	}
	m_position += m_camera.up() * vertical_direction * m_speed * delta_time;

	Vector2i32 mouse_delta = window->get_mouse_delta();
	if (m_mouseDown && mouse_delta.length() > 0) {
		m_rotation.x -= mouse_delta.y * m_sensitivity;
		m_rotation.y += mouse_delta.x * m_sensitivity;

		toki::clamp(m_rotation.x, -89.0f, +89.0f);

		m_camera.set_rotation(m_rotation);
	}

	m_camera.set_position(m_position);
}

void FreeFlightCameraController::on_event(toki::Event& event) {
	switch (event.type()) {
		case EventType::MOUSE_PRESS:
			m_mouseDown = true;
			break;
		case EventType::MOUSE_RELEASE:
			m_mouseDown = false;
			break;
		default:
			break;
	}
}

}  // namespace toki
