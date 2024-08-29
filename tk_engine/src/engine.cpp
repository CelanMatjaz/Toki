#include "engine.h"

#include "core/logging.h"

namespace Toki {

void engine_initialize() {
    const uint32_t log_flags = Core::LOG_CONSOLE | Core::LOG_FILE;
    Core::logging_initialize(log_flags, "toki.log");
    TK_LOG_INFO("Initializing engine...");
}

void engine_shutdown() {
    TK_LOG_INFO("Shutting down engine...");
    Core::logging_shutdown();
}

}  // namespace Toki
