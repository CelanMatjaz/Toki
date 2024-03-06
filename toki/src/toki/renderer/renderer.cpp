#include "renderer.h"

#include "renderer/vulkan_renderer.h"

namespace Toki {

Ref<Renderer> Renderer::create() {
    return createRef<VulkanRenderer>();
}

}  // namespace Toki
