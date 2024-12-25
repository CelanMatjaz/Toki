#pragma once

#include "renderer.h"

namespace toki {

TkError create_instance(Renderer::RendererState* state);
TkError create_device(Renderer::RendererState* state);

}  // namespace toki
