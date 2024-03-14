#include "ui_window.h"

#include <print>

#include "toki/renderer/renderer_api/renderer_2d.h"
#include "toki/systems/font_system.h"

namespace Toki {

static Ref<Font> font;

static bool rectHasPoint(const glm::vec2& rectPos, const glm::vec2& rectSize, const glm::vec2& point) {
    return rectPos.x <= point.x && rectPos.y <= point.y && rectPos.x + rectSize.x >= point.x && rectPos.y + rectSize.y >= point.y;
}

UIWindow::UIWindow(const glm::vec2& position, const glm::vec2& size, glm::vec4 color) : m_position(position), m_size(size), m_color(color) {}

bool UIWindow::onEvent(Event& e) {
    EventData eventData = e.getData();
    EventType eventType = e.getType();

    static glm::vec2 lastClickPos{};

    switch (eventType) {
        case EventType::MousePress:
            lastClickPos = { eventData.i16[0], eventData.i16[1] };
            if (rectHasPoint(m_position + m_size - 10.0f, glm::vec2{ 10.0f }, lastClickPos)) {
                m_resizing = true;
                e.setHandled();
                return true;
            } else if (rectHasPoint(m_position, m_size, lastClickPos)) {
                m_moving = true;
                m_change = 0.2f;
                e.setHandled();
                return true;
            }
            break;
        case EventType::MouseRelease:
            lastClickPos = { -1.0f, -1.0f };
            m_moving = false;
            m_resizing = false;
            m_change = 0.0f;
            break;
        case EventType::MouseMove: {
            if (m_resizing) {
                glm::vec2 offset = glm::vec2{ eventData.i32[0], eventData.i32[1] };

                m_size += offset;
                e.setHandled();
            } else if (m_moving) {
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
    c.g += m_change;
    Renderer2D::drawQuad(m_position, m_size, c);
    Renderer2D::drawQuad(m_position + 1.0f, glm::vec2{ m_size.x - 2.0f, 20.0f }, glm::vec4{ 0.0f });
    Renderer2D::drawQuad(m_position + m_size - 10.0f, glm::vec2{ 10.0f }, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

    auto f = FontSystem::loadBitmapFont("test", { Toki::ResourceType::Font, "assets/fonts/calibril.ttf" });

    Renderer2D::drawFont(m_position + 4.0f, f->fontVersions[0], "Text test");
}

}  // namespace Toki
