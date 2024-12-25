#include <toki/engine.h>

int main() {
    using namespace toki;

    TK_LOG_INFO("test {} {}", 1, "test1");

    TK_ASSERT(1 == 2, "bad condition");

    Engine::Config config{};
    Engine engine(config);
    engine.run();

    return 0;
}
