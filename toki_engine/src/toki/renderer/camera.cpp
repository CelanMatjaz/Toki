#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace toki {

static glm::vec3 s_up = glm::vec3(0.0f, 1.0f, 0.0f);

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) {
    set_ortho_projection(left, right, bottom, top);
}

void OrthographicCamera::set_position(glm::vec3 position) {
    m_position = position;
    m_dirty = true;
}

void OrthographicCamera::set_rotation(f32 rotation) {
    m_rotation = rotation;
    m_dirty = true;
}

void OrthographicCamera::set_ortho_projection(float left, float right, float bottom, float top) {
    m_projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_dirty = true;
}

const glm::mat4& OrthographicCamera::get_view_projection_matrix() {
    if (m_dirty) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) * glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        m_view = glm::inverse(transform);
        m_viewProjection = m_projection * m_view;

        m_dirty = false;
    }

    return m_viewProjection;
}

}  // namespace toki
