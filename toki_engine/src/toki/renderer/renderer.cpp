#include "renderer.h"

#include "renderer/vulkan/vulkan_renderer.h"

namespace toki {

std::shared_ptr<Renderer> Renderer::create(const Config& config) {
    return std::make_shared<VulkanRenderer>(config);
}

Renderer::Renderer(const Config& config) {}

std::shared_ptr<RendererApi> Renderer::get_renderer_api() const {
    return m_rendererApi;
}

}  // namespace toki
