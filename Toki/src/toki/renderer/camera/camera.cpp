#include "tkpch.h"
#include "camera.h"

namespace Toki {

    Camera::Camera() {}

    Camera::Camera(glm::mat4 projection, glm::mat4 view) : projectionMatrix(projection), viewMatrix(view) {

    }

}