#pragma once

#include <toki/core.h>

namespace toki {

class Camera {
public:
	Camera() = default;
	Camera(float left, float right, float bottom, float top);
	~Camera() = default;

	void add_position(glm::vec3 position);
	void set_position(glm::vec3 position);
	void set_rotation(f32 rotation);

	void set_ortho_projection(float left, float right, float bottom, float top);
	void set_perspective_projection(float fovy, float aspect, float near, float far);

	const glm::mat4& get_view_projection_matrix();

private:
	glm::mat4 m_projection;
	glm::mat4 m_view;
	glm::mat4 m_viewProjection;

	glm::vec3 m_position;
	f32 m_rotation;

	b8 m_dirty;
};

}  // namespace toki
