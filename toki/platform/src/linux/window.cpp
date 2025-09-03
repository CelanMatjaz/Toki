#include "toki/platform/window.h"

#if !defined(TK_WINDOW_SYSTEM_GLFW)

namespace toki {

void window_system_initialize(const toki::WindowSystemConfig& config) {}
void window_system_shutdown() {}

Window::Window() {}

}  // namespace toki

#endif
