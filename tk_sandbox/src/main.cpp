#include <toki/engine.h>

int main() {

    TK_LOG_INFO("LOG INFO");
    TK_LOG_WARN("LOG WARN");
    TK_LOG_ERROR("LOG ERROR");
    TK_LOG_FATAL("LOG FATAL");

    Toki::engine_initialize();
    Toki::engine_shutdown();
}
