#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace Toki {

class Camera {
public:
    Camera();
    Camera(glm::mat4 projection, glm::mat4 view);
    ~Camera() = default;

    [[NODISCARD]] const glm::mat4& getView() const;
    [[NODISCARD]] const glm::mat4& getProjection() const;

    void setProjection(const glm::mat4& newProjection);
    void setView(const glm::mat4& newView);

protected:
    glm::mat4 m_viewMatrix{ 1.0f };
    glm::mat4 m_projectionMatrix{ 1.0f };
};

}  // namespace Toki
