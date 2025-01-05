#include "renderer.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <print>

#include "core/logging.h"
#include "device.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/data/render_pass.h"
#include "renderer/vulkan/renderer_api.h"
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

    for (auto& [_, shader] : m_shaderMap) {
        GraphicsPipeline::cleanup(m_context, shader);
    }
    m_shaderMap.clear();

    for (auto& [_, render_pass] : m_context->renderPassMap) {
        RenderPass::cleanup(m_context, render_pass.renderPass);
    }

    m_windows.clear();

    vkDestroyDevice(m_context->device, m_context->allocationCallbacks);
    vkDestroyInstance(m_context->instance, m_context->allocationCallbacks);
}

Handle VulkanRenderer::create_shader(const Shader::Config& config) {
    Handle handle;

    GraphicsPipeline::Config graphics_pipeline_config{};
    graphics_pipeline_config.vertex_shader_path = config.vertex_shader_path;
    graphics_pipeline_config.fragment_shader_path = config.fragment_shader_path;
    graphics_pipeline_config.attachments = config.attachments;
    m_shaderMap.emplace(handle, GraphicsPipeline::create(m_context, graphics_pipeline_config));

    return handle;
}

void VulkanRenderer::begin_frame() {}

void VulkanRenderer::end_frame() {}

void VulkanRenderer::present() {}

}  // namespace toki
