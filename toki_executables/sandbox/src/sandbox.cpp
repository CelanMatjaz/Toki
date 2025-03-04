#include <filesystem>

#include "core/logging.h"
#include "test_view.h"
#include "toki.h"

int main(int argc, char* argv[]) {
    using namespace toki;

    if (argc < 2) {
        TK_LOG_FATAL("No working directory path provided");
        exit(1);
    }
    std::filesystem::current_path(argv[1]);

    Window::Config window_config{};
    window_config.width = 600;
    window_config.height = 600;
    window_config.title = "Window";

    Engine::Config engine_config{};
    engine_config.window_config = window_config;
    Engine engine(engine_config);
    engine.add_view(new TestView());
    engine.run();

    return 0;
}
