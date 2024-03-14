#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "toki/events/event.h"

namespace Toki {

class UIWindow {
public:
    UIWindow() = default;
    UIWindow(const glm::vec2& position, const glm::vec2& size, glm::vec4 color = glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f });

    bool onEvent(Event& e);
    void draw();

private:
    glm::vec2 m_position = glm::vec2{ 0.0f, 0.0f };
    glm::vec2 m_size = glm::vec2{ 100.0f, 100.0f };
    glm::vec4 m_color = glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f };
    float change = 0.0f;
};

}  // namespace Toki
