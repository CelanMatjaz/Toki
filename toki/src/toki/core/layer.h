#pragma once

#include "toki/events/event.h"
#include "toki/renderer/renderer.h"

namespace Toki {

class Layer {
public:
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onRender() {}
    virtual void onUpdate(const float deltaTime) {}
    virtual void onEvent([[maybe_unused]] Event& event) {}
};

}  // namespace Toki
