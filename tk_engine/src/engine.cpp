#include "engine.h"

#include <GLFW/glfw3.h>
#include <toki/renderer.h>

#include <vector>

#include "core/logging.h"
#include "core/window.h"

namespace Toki {

EngineState engine_initialize(const EngineConfig& engine_config) {
    glfwInit();

    EngineState engine_state{};

    const uint32_t log_flags = TK_LOG_FLAGS_CONSOLE | TK_LOG_FLAGS_FILE;
    engine_logging_initialize(log_flags, "toki.log");

    engine_state.windows.emplace_back(engine_create_window(engine_config.window_config));

    renderer_initialize(engine_state.windows[0].glfw_window);

    return engine_state;
}

void engine_shutdown(EngineState& engine_state) {
    renderer_shutdown();
    engine_logging_shutdown();

    glfwTerminate();
}

void engine_run(EngineState& engine_state) {
    auto should_windows_close = [](EngineState& engine_state) {
start_loop:
        for (auto it = engine_state.windows.begin(); it != engine_state.windows.end(); ++it) {
            if (glfwWindowShouldClose(it->glfw_window)) {
                engine_destroy_window(*it);
                engine_state.windows.erase(it);
                goto start_loop;
            }
        }

        if (engine_state.windows.size() == 0) {
            engine_state.running = false;
        }
    };

    while (engine_state.running) {
        glfwPollEvents();

        should_windows_close(engine_state);
    }
}

}  // namespace Toki
