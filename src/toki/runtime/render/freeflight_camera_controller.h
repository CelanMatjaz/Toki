#pragma once

#include <toki/runtime/render/camera.h>

namespace toki {

class FreeFlightCameraController {
public:
	Camera& camera();

	void on_update(f32 delta_time, const Window* window);
	void on_event(toki::Event& event);

	const Vector3& position() const;

private:
	Camera m_camera;
	Vector3 m_position{};
	Vector3 m_rotation{};
	f32 m_speed		  = 1.0f;
	f32 m_sensitivity = 0.5f;
	b8 m_mouseDown	  = false;
};

}  // namespace toki
