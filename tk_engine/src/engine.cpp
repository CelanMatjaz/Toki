#include "engine.h"

#include "core/logging.h"

namespace Toki {

void engine_initialize() {
    Core::logging_initialize(Core::LOG_CONSOLE);
    TK_LOG_INFO("Initializing engine...");
}

void engine_shutdown() {
    TK_LOG_INFO("Shutting down engine...");
    Core::logging_shutdown();
}

}  // namespace Toki
