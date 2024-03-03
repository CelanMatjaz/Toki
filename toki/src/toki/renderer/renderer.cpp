#include "renderer.h"

#include "renderer/vulkan_renderer.h"

namespace Toki {

Scope<Renderer> Renderer::create() {
    return createScope<VulkanRenderer>();
}

}  // namespace Toki
