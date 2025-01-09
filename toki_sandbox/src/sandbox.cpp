#include <filesystem>

#include "test_view.h"
#include "toki.h"

int main(int argc, char* argv[]) {
    using namespace toki;

    std::filesystem::current_path(argv[1]);

    Window::Config window_config{};
    window_config.width = 800;
    window_config.height = 600;
    window_config.title = "Window";

    Engine::Config engine_config{};
    engine_config.window_config = window_config;
    Engine engine(engine_config);
    engine.add_view(create_ref<TestView>());
    engine.run();

    return 0;
}
