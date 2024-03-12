#pragma once

#include <toki/renderer/render_pass.h>

#include "toki/events/event.h"
#include "toki/renderer/renderer.h"

namespace Toki {

class Application;

class Layer {
    friend Application;

public:
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onRender() {}
    virtual void onUpdate(const float deltaTime) {}
    virtual void onEvent(Event& event) {}

protected:
    void submit(Ref<RenderPass> renderPass, RendererSubmitFn submitFn);
    Ref<Window> m_window;

private:
    inline static Ref<Renderer> s_renderer;
};

}  // namespace Toki
