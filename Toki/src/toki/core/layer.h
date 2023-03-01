#pragma once

#include "toki/events/event.h"

namespace Toki {

    class Layer {
    public:
        virtual ~Layer() = default;
        
        virtual void onEvent(Event& e) {};
        virtual void onAttach() {};
        virtual void onDetach() {};
        virtual void onUpdate(float deltaTime) {};
    };

}