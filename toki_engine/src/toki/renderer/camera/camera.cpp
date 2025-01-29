#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <print>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/matrix.hpp"

namespace toki {

Camera::Camera(float left, float right, float bottom, float top) {
    set_ortho_projection(left, right, bottom, top);
}

void Camera::add_position(glm::vec3 position) {
    m_position += position;
    m_dirty = true;
}

void Camera::set_position(glm::vec3 position) {
    m_position = position;
    m_dirty = true;
}

void Camera::set_rotation(f32 rotation) {
    m_rotation = rotation;
    m_dirty = true;
}

void Camera::set_ortho_projection(float left, float right, float bottom, float top) {
    m_projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_projection[1][1] *= -1;
    m_dirty = true;
}

void Camera::set_perspective_projection(float fovy, float aspect, float near_, float far_) {
    m_projection = glm::perspective(fovy, aspect, near_, far_);
    m_projection[1][1] *= -1;
    m_dirty = true;
}

const glm::mat4& Camera::get_view_projection_matrix() {
    if (m_dirty) {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
        m_view = translation;
        m_viewProjection = m_projection * m_view;

        m_dirty = false;
    }

    return m_viewProjection;
}

}  // namespace toki
