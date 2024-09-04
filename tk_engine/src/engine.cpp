#include "engine.h"

#include <GLFW/glfw3.h>
#include <toki/core.h>
#include <toki/renderer.h>

#include <print>
#include <vector>

#include "core/logging.h"
#include "core/window.h"

#define CHECK_ERROR(error)                                                       \
    if (!error) {                                                                 \
        std::println("Error: {}, code: {}", (uint64_t) error.error, error.code); \
    }

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

    Window window = engine_create_window(engine_config.window_config);
    s_engine_state->windows.emplace_back(window);

    // Renderer
    RendererInitConfig renderer_init_config{};
    renderer_init_config.initial_window = s_engine_state->windows[0].glfw_window;
    TkError error = renderer_initialize(renderer_init_config);
    CHECK_ERROR(error);
}

void engine_shutdown() {
    TK_ASSERT(s_engine_state != nullptr, "Engine state is not initialized");

    TkError error = renderer_shutdown();
    CHECK_ERROR(error);

    engine_logging_shutdown();

    glfwTerminate();

    delete s_engine_state;
}

void should_windows_close(EngineState* engine_state);

void engine_run() {
    while (s_engine_state->running) {
        glfwPollEvents();
        should_windows_close(s_engine_state);
    }
}

void should_windows_close(EngineState* engine_state) {
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
}

}  // namespace Toki
