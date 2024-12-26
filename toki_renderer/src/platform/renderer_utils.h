#pragma once

#include <GLFW/glfw3.h>

#include "renderer.h"

namespace toki {

TkError create_surface(RendererState* state, GLFWwindow* window);

}
