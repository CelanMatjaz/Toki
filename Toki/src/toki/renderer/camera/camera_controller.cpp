#include "tkpch.h"
#include "camera_controller.h"
#include "input/input.h"
#include "core/engine.h"

namespace Toki {

    CameraController::CameraController(glm::mat4 projection, glm::mat4 view) : Camera(projection, view) {

    }

    CameraController::CameraController(float fov, float aspect, float nearP, float farP) {
        projectionMatrix = glm::perspective(fov, aspect, nearP, farP);
        projectionMatrix[1][1] *= -1;

        updateView();
    }

    CameraController::CameraController(float left, float right, float bottom, float top, float nearP, float farP) {
        projectionMatrix = glm::ortho(left, right, bottom, top, nearP, farP);
    }

    void CameraController::onUpdate(float deltaTime) {
        static const float cameraSpeed = 5.0f;
        if (Input::isKeyPressed(ScanCode::SCAN_W)) {
            position += cameraSpeed * front * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_S)) {
            position -= cameraSpeed * front * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_A)) {
            position -= left * cameraSpeed * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_D)) {
            position += left * cameraSpeed * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_LEFT_SHIFT)) {
            position -= up * cameraSpeed * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_LEFT_CONTROL)) {
            position += up * cameraSpeed * deltaTime;
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_LEFT)) {
            rotation.y -= glm::radians(50.0f * deltaTime);
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_RIGHT)) {
            rotation.y += glm::radians(50.0f * deltaTime);
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_UP)) {
            rotation.x -= glm::radians(50.0f * deltaTime);
            isDirty = true;
        }
        if (Input::isKeyPressed(ScanCode::SCAN_DOWN)) {
            rotation.x += glm::radians(50.0f * deltaTime);
            isDirty = true;
        }

        if (isDirty)
            updateView();
    }

    void CameraController::updateView() {
        glm::mat4 rotation = glm::eulerAngleXYZ(this->rotation.x, this->rotation.y, this->rotation.z);
        glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, position);
        front = glm::vec3{ glm::inverse(viewMatrix)[2] };
        left = -glm::vec3{ glm::inverse(viewMatrix)[0] };

        viewMatrix = rotation * translation;

        isDirty = false;
    }

    void CameraController::setPosition(const glm::vec3& position) {
        this->position = position;
        updateView();
    }

}