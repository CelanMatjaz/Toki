#pragma once

#include "core.h"
#include "window.h"
#include "layer.h"
#include "vector"
#include "chrono"
#include "renderer/imgui/imgui_layer.h"

namespace Toki {

    class Window;

    struct EngineConfig {
        std::filesystem::path workingDirectory;
        struct WindowConfig {
            std::string title = "Window";
            uint32_t width = 1280;
            uint32_t height = 720;
            bool resizable = false;
        } windowConfig;
    };

    class Engine {
    public:
        Engine(const EngineConfig& config);
        ~Engine();

        void run();

        void pushLayer(Ref<Layer> layer);
        void popLayer();

        void onEvent(Event& event);

        static Ref<Window> getWindow() { return window; }

    private:
        static inline Engine* engine;
        static inline Ref<Window> window;
        std::vector<Ref<Layer>> layers;
        Ref<ImGuiLayer> imguiLayer;

        std::chrono::steady_clock::time_point lastFrameTime;
    };

}