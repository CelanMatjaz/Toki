#pragma once

#include "glm/glm.hpp"

namespace Toki {

    class Camera {
    public:
        Camera();
        Camera(glm::mat4 projection, glm::mat4 view);
        ~Camera() = default;
        const glm::mat4& getView() { return viewMatrix; }
        const glm::mat4& getProjection() { return projectionMatrix; }

        void setProjection(glm::mat4 projection) {
            projectionMatrix = projection;
            projectionMatrix[1][1] *= -1;
        }

    protected:
        glm::mat4 viewMatrix{ 1.0f };
        glm::mat4 projectionMatrix{ 1.0f };
    };

}