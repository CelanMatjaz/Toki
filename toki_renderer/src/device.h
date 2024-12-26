#pragma once

#include <GLFW/glfw3.h>
#include <toki/core.h>

#include "renderer_state.h"

namespace toki {

TkError create_instance(RendererState* state);
TkError create_device(RendererState* state);
TkError create_surface(RendererState* state, GLFWwindow* window);

bool check_validation_layel_support();

}  // namespace toki
