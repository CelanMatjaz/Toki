#include "renderer.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <print>

#include "core/logging.h"
#include "device.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/data/vulkan_shader.h"
#include "renderer/vulkan/swapchain.h"

namespace toki {

VulkanRenderer::VulkanRenderer(const Config& config): Renderer(config) {
    TK_LOG_INFO("Initializing renderer");
    m_context = create_ref<RendererContext>();
    m_rendererApi = create_ref<VulkanRendererApi>(m_context);

    create_instance(m_context);
    create_device(m_context, config.initialWindow);

    m_windows.emplace_back(create_ref<RendererWindow>(m_context, config.initialWindow));
}

VulkanRenderer::~VulkanRenderer() {
    TK_LOG_INFO("Shutting down renderer");

    m_windows.clear();

    vkDestroyDevice(m_context->device, m_context->allocationCallbacks);
    vkDestroyInstance(m_context->instance, m_context->allocationCallbacks);
}

Ref<Shader> VulkanRenderer::create_shader(const Shader::Config& config) const {
    TK_LOG_DEBUG("Tesst");
    return create_ref<VulkanShader>(m_context, config);
}

void VulkanRenderer::begin_frame() {}

void VulkanRenderer::end_frame() {}

void VulkanRenderer::present() {}

}  // namespace toki
