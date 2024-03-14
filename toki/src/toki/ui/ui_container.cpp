#include "ui_container.h"

#include <ranges>

#include "toki/core/assert.h"

#ifndef MAX_UI_WINDOWS
#define MAX_UI_WINDOWS 64
#endif

namespace Toki {

UIWindow& UIContainer::addWindow(const char* name, const UIWindow& newComponent) {
    TK_ASSERT(!m_windows.contains(name), std::format("UI window with name: {} already exists", name));
    TK_ASSERT(m_windows.size() <= MAX_UI_WINDOWS, std::format("Adding new UI window would exceed the limit ({})", MAX_UI_WINDOWS));
    m_windowEventHandleOrder.emplace_back(name);
    m_windows.emplace(name, newComponent);

    return m_windows[name];
}

void UIContainer::onEvent(Event& e) {
    for (auto it = m_windowEventHandleOrder.begin(); it != m_windowEventHandleOrder.end(); ++it) {
        if (m_windows[*it].onEvent(e)) {
            std::rotate(m_windowEventHandleOrder.begin(), it, it + 1);
        }
        if (e.isHandled()) break;
    }
}

void UIContainer::draw() {
    for (auto it = m_windowEventHandleOrder.rbegin(); it != m_windowEventHandleOrder.rend(); ++it) {
        m_windows[*it].draw();
    }
}

}  // namespace Toki
