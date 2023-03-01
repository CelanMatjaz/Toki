#pragma once

#include "tkpch.h"
#include "layer.h"

namespace Toki {

    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        void pushLayer(Layer* layer);
        void popLayer();

        std::vector<Layer*> layers;
    };

}