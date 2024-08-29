#pragma once

#include <vector>
#include "core/window.h"

namespace Toki {

    struct EngineConfig {
WindowConfig window_config;
    };

struct EngineState {
    bool running = true;
    std::vector<Window> windows;
};

EngineState engine_initialize(const EngineConfig& engine_config);
void engine_shutdown( EngineState& engine_state);
void engine_run( EngineState& engine_state);

}  // namespace Toki
