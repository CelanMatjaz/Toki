#pragma once

#include "core/core.h"
#include "core/window.h"
#include "events/events.h"

namespace Toki {

class Renderer {
public:
    Renderer(Ref<Window> window) : window(window) {}
    virtual ~Renderer() = default;

    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void onEvent(Event& event) = 0;

protected:
    Ref<Window> window;
    inline static Scope<Renderer> renderer;
};

}  // namespace Toki
