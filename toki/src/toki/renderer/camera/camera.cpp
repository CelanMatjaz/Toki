#include "camera.h"

namespace Toki {

Camera::Camera() {}

Camera::Camera(glm::mat4 projection, glm::mat4 view) : m_projectionMatrix(projection), m_viewMatrix(view) {}

const glm::mat4& Camera::getView() const {
    return m_viewMatrix;
}

const glm::mat4& Camera::getProjection() const {
    return m_projectionMatrix;
}

void Camera::setProjection(const glm::mat4& newProjection) {
    m_projectionMatrix = newProjection;
    m_projectionMatrix[1][1] *= -1;
}

void Camera::setView(const glm::mat4& newView) {
    m_viewMatrix = newView;
}

}  // namespace Toki