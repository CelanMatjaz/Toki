#include "engine.h"

#include <GLFW/glfw3.h>
#include <toki/renderer.h>

#include "core/logging.h"
#include "core/window.h"

namespace Toki {

void engine_initialize() {
    glfwInit();

    const uint32_t log_flags = TK_LOG_FLAGS_CONSOLE | TK_LOG_FLAGS_FILE;
    engine_logging_initialize(log_flags, "toki.log");

    WindowConfig window_config{};
    window_config.title = "Window";
    window_config.width = 400;
    window_config.height = 300;

    Window window = engine_create_window(window_config);

    renderer_initialize(window.glfw_window);

    while (!glfwWindowShouldClose(window.glfw_window)) {
        glfwPollEvents();
    }
}

void engine_shutdown() {
    renderer_shutdown();
    engine_logging_shutdown();

    glfwTerminate();
}

}  // namespace Toki
