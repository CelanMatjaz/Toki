#include <toki/core.h>
#include <toki/engine.h>
#include <toki/window.h>

int tk_entry_point(int, char**) {
    toki::Engine::Config engine_config{};
    engine_config.memory_config.engine_memory_block_size = toki::GB(8);
    engine_config.memory_config.engine_frame_memory_block_size = toki::MB(128);
    engine_config.memory_config.renderer_frame_memory_block_size = toki::MB(128);

    auto window = toki::WindowSystem::create_window("Test", 400, 400);

    while (true) {
        toki::WindowSystem::poll_events();
    }

    toki::Engine engine(engine_config);
    engine.window_add("Test", 800, 600);
    engine.run();

    toki::WindowSystem::destroy_window(window);

    return 0;
}
