#include "test_view.h"
#include "toki.h"

int main() {
    using namespace toki;

    Window::Config window_config{};
    window_config.width = 800;
    window_config.height = 600;
    window_config.title = "Window";
    auto window = Window::create(window_config);

    Engine::Config engine_config{};
    engine_config.initialWindow = window;
    Engine engine(engine_config);
    engine.add_view(create_ref<TestView>());
    engine.run();

    return 0;
}
