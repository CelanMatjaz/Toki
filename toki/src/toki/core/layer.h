#pragma once

#include "events/events.h"

namespace Toki {

class Layer {
public:
    Layer() = default;
    virtual ~Layer() = default;

    virtual void onAttach() {}

    virtual void onDetach() {}

    virtual void onRender() {}

    virtual void onUpdate(const float deltaTime) {}

    virtual void onEvent(Event& event) {}
};

}  // namespace Toki
