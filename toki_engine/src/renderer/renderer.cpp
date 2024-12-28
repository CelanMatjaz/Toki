#include "renderer.h"

#include <vulkan/vulkan.h>

#include <print>

#include "core/logging.h"
#include "device.h"

namespace toki {

Renderer::Renderer(const Config& config) {
    TK_LOG_INFO("Initializing renderer");

    create_instance(&m_context);
    create_device(&m_context, config.initialWindow);
}

Renderer::~Renderer() {
    TK_LOG_INFO("Shutting down renderer");

    vkDestroyDevice(m_context.device, m_context.allocationCallbacks);
    vkDestroyInstance(m_context.instance, m_context.allocationCallbacks);
}

}  // namespace toki
