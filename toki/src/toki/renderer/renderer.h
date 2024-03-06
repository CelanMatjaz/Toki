#pragma once

#include <functional>

#include "toki/core/core.h"
#include "toki/core/window.h"
#include "toki/renderer/render_pass.h"

namespace Toki {

using RendererSubmitFn = std::function<void(const RenderingContext)>;

class Renderer {
public:
    static Ref<Renderer> create();

    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void submit(Ref<RenderPass> renderPass, RendererSubmitFn submitFn) = 0;

    virtual void createSwapchain(Ref<Window> window) = 0;
};

}  // namespace Toki