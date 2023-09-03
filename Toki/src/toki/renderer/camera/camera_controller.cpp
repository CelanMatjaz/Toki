#include "tkpch.h"
#include "camera_controller.h"

#include "GLFW/glfw3.h"
#include "core/engine.h"

namespace Toki {

    CameraController::CameraController(glm::mat4 projection, glm::mat4 view) : Camera(projection, view) {

    }

    CameraController::CameraController(float fov, float aspect, float near, float far) {
        projectionMatrix = glm::perspective(fov, aspect, near, far);
        projectionMatrix[1][1] *= -1;

        updateView();
    }

    CameraController::CameraController(float left, float right, float bottom, float top, float near, float far) {
        projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
    }

    void CameraController::onUpdate() {
        GLFWwindow* window = (GLFWwindow*) Engine::getWindow()->getHandle();

        static const float cameraSpeed = 0.05f; // adjust accordingly
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position += cameraSpeed * front;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position -= cameraSpeed * front;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position -= left * cameraSpeed;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position += left * cameraSpeed;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            position -= up * cameraSpeed;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            position += up * cameraSpeed;
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            rotation.y -= glm::radians(1.0f);
            isDirty = true;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            rotation.y += glm::radians(1.0f);
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

}