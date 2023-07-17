#pragma once

#include "tkpch.h"

namespace Toki {

    class Camera {
    public:
        enum CameraProjection {
            PERSPECTIVE, ORTHOGRAPHIC
        };

        enum CameraType {
            FIRST_PERSON, LOOK_AT
        };

        glm::vec3 rotation { 0, 0, 0 };
    public:
        Camera();
        virtual ~Camera() = default;
        Camera(CameraProjection cameraProjection, CameraType cameraType, const glm::vec3& position = { 0.0f, 0.0f, 0.0f });

        void setOrtho(float left, float right, float bottom, float top, float near, float far);
        void setPerspective(float fov, float aspect, float near, float far);

        void setView(const glm::mat4& newView);
        void setProjection(CameraProjection projection, const glm::mat4& newProjection);

        void lookAt(const glm::vec3& lookAtPosition);
        void lookAt(const glm::vec3& lookAtPosition, const glm::vec3& cameraPosition);

        void setPosition(glm::vec3 newPosition);

        void moveForward(float amount);
        void moveUp(float amount);
        void moveLeft(float amount);

        void rotateLeft(float amount);
        void rotateUp(float amount);

        const glm::mat4& getView() { return viewMatrix; }
        const glm::mat4& getProjection() { return projectionMatrix; }

        void updateViewMatrix();
    protected:
        void updateProjectionMatrix();

        glm::mat4 viewMatrix{ 1.0f };
        glm::mat4 projectionMatrix{ 1.0f };

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };

        CameraProjection projection;
        CameraType type;
    };

}