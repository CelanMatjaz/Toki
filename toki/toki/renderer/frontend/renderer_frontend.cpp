#include "toki/renderer/frontend/renderer_frontend.h"

#include <toki/core/core.h>
#include <toki/renderer/private/vulkan/vulkan_backend.h>

namespace toki {

toki::UniquePtr<Renderer> Renderer::create(const RendererConfig& config) {
	return make_unique<Renderer>(config);
}

Renderer::Renderer(const RendererConfig& config) {
	VulkanBackend* ptr = reinterpret_cast<VulkanBackend*>(
		RendererPersistentAllocator::allocate_aligned(sizeof(VulkanBackend), alignof(VulkanBackend)));
	m_internalData = toki::construct_at<VulkanBackend>(ptr, config);
}

}  // namespace toki
