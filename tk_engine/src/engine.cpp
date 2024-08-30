#include "engine.h"

#include <GLFW/glfw3.h>
#include <toki/core.h>
#include <toki/renderer.h>

#include <vector>

#include "core/logging.h"
#include "core/window.h"

namespace Toki {

struct EngineState {
    bool running = true;
    std::vector<Window> windows;
} static* s_engine_state;

void engine_initialize(const EngineConfig& engine_config) {
    TK_ASSERT(s_engine_state == nullptr, "Engine state is already initialized");
    s_engine_state = new EngineState();

    glfwInit();

    // Engine systems
    const uint32_t log_flags = TK_LOG_FLAGS_CONSOLE | TK_LOG_FLAGS_FILE;
    engine_logging_initialize(log_flags, "toki.log");

    s_engine_state->windows.emplace_back(engine_create_window(engine_config.window_config));

    // Renderer
    RendererStateConfig renderer_state_config{};
    renderer_initialize_state(renderer_state_config);
}

void engine_shutdown() {
    TK_ASSERT(s_engine_state != nullptr, "Engine state is not initialized");

    renderer_destroy_state();
    engine_logging_shutdown();
    glfwTerminate();

    delete s_engine_state;
}

void engine_run() {
    auto should_windows_close = [](EngineState* engine_state) {
start_loop:
        for (auto it = engine_state->windows.begin(); it != engine_state->windows.end(); ++it) {
            if (glfwWindowShouldClose(it->glfw_window)) {
                engine_destroy_window(*it);
                engine_state->windows.erase(it);
                goto start_loop;
            }
        }

        if (engine_state->windows.size() == 0) {
            engine_state->running = false;
        }
    };

    while (s_engine_state->running) {
        glfwPollEvents();
        should_windows_close(s_engine_state);
    }
}

}  // namespace Toki
