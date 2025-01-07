#include "window.h"

#include "core/logging.h"
#include "platform/glfw_window.h"

namespace toki {

window::window(const config& config) {}

window::~window() {}

ref<window> window::create(const config& config) {
    TK_LOG_INFO("Creating new window");
    return std::make_shared<glfw_window>(config);
}

}  // namespace toki
