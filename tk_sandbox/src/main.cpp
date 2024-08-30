#include <toki/engine.h>

int main() {
    Toki::WindowConfig window_config{};
    window_config.title = "Window";
    window_config.width = 400;
    window_config.height = 300;

    Toki::EngineConfig engine_config{};
    engine_config.window_config = window_config;

    Toki::engine_initialize(engine_config);
    Toki::engine_run();
    Toki::engine_shutdown();
}
