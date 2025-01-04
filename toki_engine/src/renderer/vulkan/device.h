#pragma once

#include <GLFW/glfw3.h>

#include "core/error.h"
#include "engine/window.h"
#include "renderer_state.h"

namespace toki {

TkError create_instance(RendererContext* state);
TkError create_device(RendererContext* ctx, std::shared_ptr<Window> window);

}  // namespace toki
