#include "renderer.h"

#include <vulkan/vulkan.h>

#include <print>

#include "core/logging.h"
#include "device.h"
#include "renderer/swapchain.h"

namespace toki {

Renderer::Renderer(const Config& config) {
    TK_LOG_INFO("Initializing renderer");

    create_instance(&m_context);
    create_device(&m_context, config.initialWindow);

    m_swapchains.emplace_back(
        Swapchain::create(&m_context, (GLFWwindow*) config.initialWindow->get_handle()));
}

Renderer::~Renderer() {
    TK_LOG_INFO("Shutting down renderer");

    for (auto& swapchain : m_swapchains) {
        swapchain->destroy(&m_context);
    }

    vkDestroyDevice(m_context.device, m_context.allocationCallbacks);
    vkDestroyInstance(m_context.instance, m_context.allocationCallbacks);
}

}  // namespace toki
