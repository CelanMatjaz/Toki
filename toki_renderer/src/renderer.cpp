
#include "renderer.h"

#include <memory>
#include <print>

namespace toki {

Renderer::Renderer(const Config& config):
    m_state(std::make_unique<RendererState>()) {
    TK_LOG_INFO("Initializing renderer");
}

Renderer::~Renderer() {
    TK_LOG_INFO("Shutting down renderer");
}

}  // namespace toki
