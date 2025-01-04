#include "renderer.h"

#include <vulkan/vulkan.h>

#include <print>

#include "core/logging.h"
#include "device.h"
#include "platform/windows/glfw_window.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/swapchain.h"

namespace toki {

VulkanRenderer::VulkanRenderer(const Config& config): Renderer(config) {
    TK_LOG_INFO("Initializing renderer");
    m_context = std::make_unique<RendererContext>();

    create_instance(m_context.get());
    create_device(m_context.get(), config.initialWindow);

    m_swapchains.emplace_back(Swapchain::create(
        m_context.get(), ((GlfwWindow*) config.initialWindow.get())->get_handle()));
}

VulkanRenderer::~VulkanRenderer() {
    TK_LOG_INFO("Shutting down renderer");

    for (auto& swapchain : m_swapchains) {
        swapchain->destroy(m_context.get());
    }

    vkDestroyDevice(m_context->device, m_context->allocationCallbacks);
    vkDestroyInstance(m_context->instance, m_context->allocationCallbacks);
}

}  // namespace toki
