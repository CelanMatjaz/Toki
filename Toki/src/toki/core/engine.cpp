#include "tkpch.h"
#include "engine.h"
#include "assert.h"
#include "renderer/renderer.h"

namespace Toki {

    Engine::Engine(const EngineConfig& config) {
        std::filesystem::current_path(config.workingDirectory);

        if (!engine)
            engine = this;

        WindowConfig windowConfig{};
        window = Window::create(windowConfig, this);

        Renderer::initRenderer();
    }

    Engine::~Engine() {
        uint32_t layerCount = layers.size();
        for (uint32_t i = 0; i < layerCount; ++i) {
            popLayer();
        }

        Renderer::shutdownRenderer();
    }

    void Engine::run() {
        while (!window->shouldClose()) {
            auto now = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
            lastFrameTime = now;

            Renderer::getRenderer()->beginFrame();

            for (int32_t i = layers.size() - 1; i >= 0; --i) {
                layers[i]->onUpdate(deltaTime);
            }

            Renderer::getRenderer()->endFrame();

            window->pollEvents();
        }
    }

    void Engine::pushLayer(Ref<Layer> layer) {
        layer->onAttach();
        layers.push_back(layer);
    }

    void Engine::popLayer() {
        TK_ASSERT(layers.size(), "There are no layers on layer stack");
        layers[layers.size() - 1]->onDetach();
        layers.erase(layers.end() - 1);
    }

}