#include "tkpch.h"
#include "engine.h"
#include "assert.h"
#include "renderer/renderer.h"
#include "events/events.h"

namespace Toki {

    Engine::Engine(const EngineConfig& config) {
        std::filesystem::current_path(config.workingDirectory);

        if (!engine)
            engine = this;

        window = Window::create(*((WindowConfig*) &config.windowConfig), this);

        Renderer::initRenderer();

        imguiLayer = ImGuiLayer::create();
        imguiLayer->onAttach();
    }

    Engine::~Engine() {
        uint32_t layerCount = layers.size();
        for (uint32_t i = 0; i < layerCount; ++i) {
            popLayer();
        }

        imguiLayer->onDetach();
        imguiLayer.reset();

        Renderer::shutdownRenderer();
    }

    void Engine::run() {
        while (!window->shouldClose()) {
            auto now = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
            lastFrameTime = now;

            window->pollEvents();

            Renderer::getRenderer()->beginFrame();

            for (int32_t i = layers.size() - 1; i >= 0; --i) {
                layers[i]->onUpdate(deltaTime);
            }

            // { // ImGui rendering
            //     imguiLayer->startImGuiFrame();
            //     for (int layerIndex = layers.size() - 1; layerIndex >= 0; --layerIndex)
            //         layers[layerIndex]->renderImGui();
            //     imguiLayer->endImGuiFrame();
            // }

            Renderer::getRenderer()->endFrame();
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

    void Engine::onEvent(Event& event) {
        if (event.getType() == Event::EventType::WindowResized) {
            WindowResizeEvent* ev = (WindowResizeEvent*) &event;
            Renderer::getRenderer()->resize(ev->getWidth(), ev->getHeight());
        }

        imguiLayer->onEvent(event);

        for (int32_t i = layers.size() - 1; i >= 0 && !event.isHandled(); --i) {
            layers[i]->onEvent(event);
        }
    }

}