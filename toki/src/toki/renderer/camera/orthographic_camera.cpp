#include "orthographic_camera.h"

#include "toki/events/event.h"

namespace Toki {

OrthographicCamera::OrthographicCamera() : OrthographicCamera({ 0.0f, 800.0f, 600.0f, 0.0f }, true) {}

OrthographicCamera::OrthographicCamera(const OrthographicCameraBounds& bounds, bool syncToWindowSize) :
    Camera(
        glm::ortho(bounds.left, bounds.right, bounds.bottom, bounds.top, 0.0f, 1000.0f),
        glm::lookAt(glm::vec3{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f })),
    m_syncToWindowSize(syncToWindowSize) {
    if (syncToWindowSize) {
        Event::bindEvent(EventType::WindowResize, this, [](void* sender, void* receiver, const Event& event) {
            EventData data = event.getData();
            auto [width, height] = data.i32;
            ((OrthographicCamera*) receiver)->setProjection(glm::ortho(0.0f, (float) width, (float) height, 0.0f, 0.0f, 1000.0f));
        });
    }
}

OrthographicCamera::~OrthographicCamera() {}

}  // namespace Toki
