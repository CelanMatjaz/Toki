#include "renderer.h"

#include "renderer/vulkan/renderer.h"

namespace toki {

std::shared_ptr<Renderer> Renderer::create(const Config& config) {
    return std::make_shared<VulkanRenderer>(config);
}

Renderer::Renderer(const Config& config) {}

}  // namespace toki
