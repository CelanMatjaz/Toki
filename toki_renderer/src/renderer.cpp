
#include "renderer.h"

#include <print>

#include "device.h"
#include "vulkan/vulkan_core.h"

namespace toki {

Renderer::Renderer(const Config& config) {
    TK_LOG_INFO("Initializing renderer");

    create_instance(&m_state);
    create_device(&m_state);
}

Renderer::~Renderer() {
    TK_LOG_INFO("Shutting down renderer");

    vkDestroyDevice(m_state.device, m_state.allocation_callbacks);
    vkDestroyInstance(m_state.instance, m_state.allocation_callbacks);
}

}  // namespace toki
