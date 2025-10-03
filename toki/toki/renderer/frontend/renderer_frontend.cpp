#include "toki/renderer/frontend/renderer_frontend.h"

#include <toki/core/core.h>
#include <toki/renderer/private/vulkan/vulkan_backend.h>

namespace toki::renderer {

toki::UniquePtr<Renderer> Renderer::create(const RendererConfig& config) {
	return make_unique<VulkanBackend>(config);
}

Renderer::Renderer(const RendererConfig& config) {}

}  // namespace toki::renderer
