#pragma once

namespace Toki {

    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        // virtual void onEvent(Event& e) {}
        virtual void onAttach() {}
        virtual void onDetach() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void renderImGui() {}
    };

}