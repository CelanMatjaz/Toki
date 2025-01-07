#include <filesystem>

#include "test_view.h"
#include "toki.h"

int main(int argc, char* argv[]) {
    using namespace toki;

    std::filesystem::current_path(argv[1]);

    window::config window_config{};
    window_config.width = 800;
    window_config.height = 600;
    window_config.title = "Window";
    auto window = window::create(window_config);

    engine::config engine_config{};
    engine_config.initialWindow = window;
    engine engine(engine_config);
    engine.add_view(create_ref<test_view>());
    engine.run();

    return 0;
}
