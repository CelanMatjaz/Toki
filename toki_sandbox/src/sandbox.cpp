#include <toki/engine.h>

int main() {
    using namespace toki;

    Engine::Config config{};
    Engine engine(config);
    engine.run();

    return 0;
}
