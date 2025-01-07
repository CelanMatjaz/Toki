#pragma once

#include <memory>

#include "core/macros.h"
#include "renderer/renderer.h"
#include "renderer/renderer_api.h"

namespace toki {

class view {
public:
    ref<view> create();

    view() = default;
    ~view() = default;

    DELETE_COPY(view)
    DELETE_MOVE(view)

    virtual void on_add(const ref<renderer> renderer){};
    virtual void on_destroy(const ref<renderer> renderer){};
    virtual void on_render(const ref<renderer_api> renderer){};
    virtual void on_update(const float delta_time){};
};

}  // namespace toki
