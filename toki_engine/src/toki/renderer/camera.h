#pragma once

#include "core/base.h"
#include "glm/glm.hpp"

namespace toki {

class OrthographicCamera {
public:
    OrthographicCamera() = default;
    OrthographicCamera(float left, float right, float bottom, float top);
    ~OrthographicCamera() = default;

    void set_position(glm::vec3 position);
    void set_rotation(f32 rotation);

    void set_ortho_projection(float left, float right, float bottom, float top);

    const glm::mat4& get_view_projection_matrix();

private:
    glm::mat4 m_projection;
    glm::mat4 m_view;
    glm::mat4 m_viewProjection;

    glm::vec3 m_position;
    f32 m_rotation;

    glm::vec3 m_direction{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};

    b8 m_dirty;
};

}  // namespace toki
