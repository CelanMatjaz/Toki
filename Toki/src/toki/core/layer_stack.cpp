#include "layer_stack.h"

namespace Toki {

    LayerStack::~LayerStack() {
        uint32_t size = layers.size();
        for (uint32_t i = 0; i < size; ++i) popLayer();
    }

    void LayerStack::pushLayer(Layer* layer) {
        layer->onAttach();
        layers.emplace_back(layer);
    }

    void LayerStack::popLayer() {
        uint32_t layerIndex = layers.size() - 1;
        layers[layerIndex]->onDetach();
        delete layers[layerIndex];
        layers.erase(layers.end() - 1);
    }

}