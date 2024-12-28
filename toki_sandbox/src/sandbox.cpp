#include <toki/engine.h>

int main() {
    using namespace toki;

    Window::Config window_config{};
    window_config.width = 800;
    window_config.height = 600;
    window_config.title = "Window";
    auto window = Window::create(window_config);

    Engine::Config config{ .initialWindow = window };
    Engine engine(config);
    engine.run();

    return 0;
}
