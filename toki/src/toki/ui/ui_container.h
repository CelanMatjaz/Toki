#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "toki/events/event.h"
#include "toki/ui/ui_window.h"

namespace Toki {
class UIContainer {
public:
    UIWindow& addWindow(const char* name, const UIWindow& newComponent);

    void onEvent(Event& e);
    void draw();

private:
    std::unordered_map<const char*, UIWindow> m_windows;
    std::vector<const char*> m_windowEventHandleOrder;
};

}  // namespace Toki
