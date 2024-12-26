
#include "renderer.h"

#include <memory>
#include <print>

#include "device.h"

namespace toki {

Renderer::Renderer(const Config& config) {
    TK_LOG_INFO("Initializing renderer");

    create_instance(&m_state);
    create_device(&m_state);
}

Renderer::~Renderer() {
    TK_LOG_INFO("Shutting down renderer");
}

}  // namespace toki
