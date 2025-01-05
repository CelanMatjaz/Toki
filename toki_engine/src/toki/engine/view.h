#pragma once

#include <memory>

#include "core/macros.h"
#include "renderer/renderer.h"
#include "renderer/renderer_api.h"

namespace toki {

class View {
public:
    Ref<View> create();

    View() = default;
    ~View() = default;

    DELETE_COPY(View)
    DELETE_MOVE(View)

    virtual void on_add(const Ref<Renderer> renderer) {};
    virtual void on_destroy() {};
    virtual void on_render(const Ref<RendererApi> renderer) {};
    virtual void on_update(const float delta_time) {};

private:
};

}  // namespace toki
