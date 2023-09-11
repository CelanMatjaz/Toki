#pragma once

#include "camera.h"

namespace Toki {

    class CameraController : public Camera {
    public:
        CameraController(glm::mat4 projection = glm::mat4{ 1.0f }, glm::mat4 view = glm::mat4{ 1.0f });
        CameraController(float fov, float aspect, float near, float far);
        CameraController(float left, float right, float bottom, float top, float near, float far);

        void onUpdate(float deltaTime);

        void updateView();

        const glm::vec3& getFront() const { return front; }
        const glm::vec3& getLeft() const { return left; }
        const glm::vec3& getUp() const { return up; }

        void setPosition(const glm::vec3& position);

    private:
        glm::vec3 position{ 0.0f };
        glm::vec3 rotation{ 0.0f };

        glm::vec3 front{ 0.0f, 0.0f, -1.0f };
        glm::vec3 left{ -1.0f, 0.0f, 0.0f };
        glm::vec3 up{ 0.0f, 1.0f, 0.0f };

        bool isDirty = false;
    };

}