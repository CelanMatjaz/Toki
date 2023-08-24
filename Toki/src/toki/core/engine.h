#pragma once

#include "core.h"
#include "window.h"
#include "layer.h"
#include "vector"
#include "chrono"

namespace Toki {

    class Window;

    struct EngineConfig {
        std::filesystem::path workingDirectory;
    };

    class Engine {
    public:
        Engine(const EngineConfig& config);
        ~Engine();

        void run();

        void pushLayer(Ref<Layer> layer);
        void popLayer();

        static Ref<Window> getWindow() { return window; }

    private:
        static inline Engine* engine;
        static inline Ref<Window> window;
        std::vector<Ref<Layer>> layers;

        std::chrono::steady_clock::time_point lastFrameTime;
    };

}