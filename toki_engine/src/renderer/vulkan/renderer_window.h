#pragma once

#include "engine/window.h"
#include "renderer/vulkan/renderer_state.h"
#include "renderer/vulkan/swapchain.h"
namespace toki {

class RendererWindow {
public:
    RendererWindow() = delete;
    RendererWindow(Ref<RendererContext> ctx, Ref<Window> window);
    ~RendererWindow();

private:
    Ref<RendererContext> m_context;
    Ref<Window> m_window;
    Swapchain m_swapchain;
};

}  // namespace toki
