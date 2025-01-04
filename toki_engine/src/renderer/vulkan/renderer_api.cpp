#include "renderer_api.h"

#include "renderer/vulkan/data/vulkan_shader.h"

namespace toki {

VulkanRendererApi::VulkanRendererApi(Ref<RendererContext> context): m_context(context) {}

VulkanRendererApi::~VulkanRendererApi() {
    m_context.reset();
}

void VulkanRendererApi::reset_viewport() const {}

void VulkanRendererApi::reset_scissor() const {}

}  // namespace toki
