#include "renderer_window.h"

#include "core/base.h"

namespace toki {

RendererWindow::RendererWindow(Ref<RendererContext> ctx, Ref<Window> window): m_context(ctx), m_window(window) {
    m_swapchain = Swapchain::create(ctx, window);
}

RendererWindow::~RendererWindow() {}

}  // namespace toki
