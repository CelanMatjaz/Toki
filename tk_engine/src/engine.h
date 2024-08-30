#pragma once

#include "core/window.h"

namespace Toki {

struct EngineConfig {
    WindowConfig window_config;
};

void engine_initialize(const EngineConfig& engine_config);
void engine_shutdown();
void engine_run();

}  // namespace Toki
