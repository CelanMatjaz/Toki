#pragma once

#include "toki/renderer/camera/camera.h"

namespace Toki {

struct OrthographicCameraBounds {
    float left = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float top = 0.0f;
    // float near;
    // float far;
};

class OrthographicCamera : public Camera {
public:
    OrthographicCamera();
    OrthographicCamera(const OrthographicCameraBounds& bounds, bool syncToWindowSize);
    ~OrthographicCamera();

private:
    bool m_syncToWindowSize : 1 = false;
};

}  // namespace Toki
