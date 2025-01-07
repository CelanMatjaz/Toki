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
    auto window = Window::create(window_config);

    Engine::Config engine_config{};
    engine_config.initialWindow = window;
    Engine engine(engine_config);
    engine.add_view(createRef<TestView>());
    engine.run();

    return 0;
}
