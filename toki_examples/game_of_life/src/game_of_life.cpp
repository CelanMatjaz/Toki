#include "game_of_life_view.h"
#include "toki.h"

int main(int argc, char* argv[]) {
    using namespace toki;

    if (argc < 2) {
        TK_LOG_FATAL("No working directory path provided");
        exit(1);
    }
    std::filesystem::current_path(argv[1]);

    Window::Config window_config{};
    window_config.width = CELL_COL_COUNT * CELL_SIZE;
    window_config.height = CELL_ROW_COUNT * CELL_SIZE;
    window_config.title = "Game of life";
    window_config.flags.resizable = false;

    Engine::Config engine_config{};
    engine_config.window_config = window_config;
    Engine engine(engine_config);
    engine.add_view(create_ref<GameOfLifeView>());
    engine.run();

    return 0;
}
