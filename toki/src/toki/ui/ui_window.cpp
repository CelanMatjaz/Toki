#include "ui_window.h"

#include "toki/renderer/renderer_api/renderer_2d.h"

namespace Toki {

static bool rectHasPoint(const glm::vec2& rectPos, const glm::vec2& rectSize, const glm::vec2& point) {
    return rectPos.x <= point.x && rectPos.y <= point.y && rectPos.x + rectSize.x >= point.x && rectPos.y + rectSize.y >= point.y;
}

UIWindow::UIWindow(const glm::vec2& position, const glm::vec2& size, glm::vec4 color) : m_position(position), m_size(size), m_color(color) {}

bool UIWindow::onEvent(Event& e) {
    EventData eventData = e.getData();
    EventType eventType = e.getType();

    static bool isHolding = false;
    static glm::vec2 lastClickPos{};

    switch (eventType) {
        case EventType::MousePress:
            lastClickPos = { eventData.i16[0], eventData.i16[1] };
            if (rectHasPoint(m_position, m_size, lastClickPos)) {
                isHolding = true;
                change = 0.2f;
                e.setHandled();
                return true;
            }
            break;
        case EventType::MouseRelease:
            lastClickPos = { -1.0f, -1.0f };
            isHolding = false;
            change = 0.0f;
            break;
        case EventType::MouseMove: {
            if (isHolding) {
                glm::vec2 offset = glm::vec2{ eventData.i32[0], eventData.i32[1] };
                m_position += offset;
                e.setHandled();
            }
        }
    }

    return false;
}

void UIWindow::draw() {
    auto c = m_color;
    c.g += change;
    Renderer2D::drawQuad(m_position, m_size, c);
}

}  // namespace Toki
