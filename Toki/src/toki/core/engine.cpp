#include "tkpch.h"
#include "engine.h"
#include "assert.h"
#include "renderer/renderer.h"
#include "events/events.h"

#include "platform/windows/windows_window.h"

namespace Toki {

    Engine::Engine(const EngineConfig& config) {
        std::filesystem::current_path(config.workingDirectory);

        if (!engine)
            engine = this;

        window = Window::create(config.windowConfig, this);

        Renderer::initRenderer();

        imguiLayer = ImGuiLayer::create();
        imguiLayer->onAttach();

        window->showWindow();
        isInit = true;
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

            Renderer::getRenderer()->beginFrame();

            for (int32_t i = layers.size() - 1; i >= 0; --i) {
                layers[i]->onUpdate(deltaTime);
            }

            { // ImGui rendering
                imguiLayer->startImGuiFrame();
                for (int layerIndex = layers.size() - 1; layerIndex >= 0; --layerIndex)
                    layers[layerIndex]->renderImGui();
                imguiLayer->endImGuiFrame();
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

    void Engine::onEvent(Event& event) {
        if (!isInit) return;

        if (event.getType() == EventType::WindowResize) {
            WindowResizeEvent* e = (WindowResizeEvent*) &event;
            Renderer::getRenderer()->resizeSwapchain(e->getWidth(), e->getHeight());
        }

        imguiLayer->onEvent(event);

        if (event.isHandled()) return;

        for (int32_t i = layers.size() - 1; i >= 0; --i) {
            layers[i]->onEvent(event);
            if (event.isHandled()) return;
        }
    }

}