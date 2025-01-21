#include "renderer.h"

#include "renderer/vulkan/vulkan_renderer.h"

namespace toki {

Renderer* Renderer::create(const Config& config) {
    return new VulkanRenderer(config);
}

Renderer::Renderer(const Config& config) {}

}  // namespace toki
