#include "tkpch.h"
#include "renderer.h"
#include "core/assert.h"

#include "platform/vulkan/vulkan_renderer.h"

namespace Toki {

    Ref<Renderer> Renderer::renderer;

    void Renderer::initRenderer() {
        renderer = createRef<VulkanRenderer>();
        renderer->init();
        Renderer::isInit = true;
    }

    void Renderer::shutdownRenderer() {
        renderer->shutdown();
        Renderer::isInit = false;
    }

}