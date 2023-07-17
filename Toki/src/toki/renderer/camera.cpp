#include "camera.h"

#include "toki/core/application.h"

namespace Toki {
    Camera::Camera() : Camera{ ORTHOGRAPHIC, LOOK_AT, { 0.0f, 0.0f, -1.0f } } {}

    Camera::Camera(CameraProjection cameraProjection, CameraType cameraType, const glm::vec3& position)
        : projection{ cameraProjection }, type{ cameraType }, position{ position } {

        auto [width, height] = Application::getWindow()->getWindowDimensions();
        float aspectRatio = (float)width / height;

        switch (projection) {
            case CameraProjection::PERSPECTIVE:
                setPerspective(glm::radians(60.0f), aspectRatio, 0.1f, 1000.0f);
                break;
            case CameraProjection::ORTHOGRAPHIC:
            default: {
                float horizontal = width >> 1;
                float vertical = height >> 1;
                setOrtho(-horizontal, horizontal, -vertical, vertical, 0.1f, 1000.0f);
                break;
            }
        }
    }

    void Camera::setOrtho(float left, float right, float bottom, float top, float near, float far) {
        projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
        updateProjectionMatrix();
    }

    void Camera::setPerspective(float fov, float aspect, float near, float far) {
        projectionMatrix = glm::perspective(fov, aspect, near, far);
        updateProjectionMatrix();
    }

    void Camera::setView(const glm::mat4& newView) {
        viewMatrix = newView;
    }

    void Camera::setProjection(CameraProjection projection, const glm::mat4& newProjection) {
        this->projection = projection;
        projectionMatrix = newProjection;
        updateProjectionMatrix();
    }

    void Camera::lookAt(const glm::vec3& lookAtPosition) {
        viewMatrix = mat4_cast(glm::quatLookAtRH(glm::normalize(lookAtPosition - this->position), { 0., 1., 0. })) * glm::inverse(glm::translate(this->position));
    }

    void Camera::lookAt(const glm::vec3& lookAtPosition, const glm::vec3& cameraPosition) {
        this->position = cameraPosition;
        lookAt(lookAtPosition);
    }

    void Camera::setPosition(glm::vec3 newPosition) {
        position = newPosition;
        updateViewMatrix();
    }

    void Camera::moveForward(float amount) {
        position += amount * glm::vec3{ glm::inverse(viewMatrix)[2] };
        updateViewMatrix();
    }

    void Camera::moveUp(float amount) {
        position += glm::vec3(0.0f, 1.0f * amount, 0.0f);
        updateViewMatrix();
    }

    void Camera::moveLeft(float amount) {
        position += amount * glm::vec3{ glm::inverse(viewMatrix)[0] };
        updateViewMatrix();
    }

    void Camera::rotateLeft(float amount) {
        this->rotation.y += amount;
        updateViewMatrix();
    }

    void Camera::rotateUp(float amount) {
        this->rotation.x += amount;
        if (this->rotation.x > 2) this->rotation.x = 1.99;
        if (this->rotation.x < -2) this->rotation.x = -1.99;
        updateViewMatrix();
    }

    void Camera::updateViewMatrix() {
        glm::mat4 rotation = glm::eulerAngleXYZ(this->rotation.x, this->rotation.y, this->rotation.z);
        glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, position);

        switch (type) {
            case FIRST_PERSON:
                viewMatrix = rotation * translation;
                break;
            case LOOK_AT:
            default:
                lookAt({ 0, 0, 0 });
                break;
        }
    }

    void Camera::updateProjectionMatrix() {
        projectionMatrix[1][1] *= -1;
    }

}