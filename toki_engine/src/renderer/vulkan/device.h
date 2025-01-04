#pragma once

#include <GLFW/glfw3.h>

#include "core/error.h"
#include "engine/window.h"
#include "renderer_state.h"

namespace toki {

TkError create_instance(Ref<RendererContext> ctx);
TkError create_device(Ref<RendererContext> ctx, Ref<Window> window);

}  // namespace toki
