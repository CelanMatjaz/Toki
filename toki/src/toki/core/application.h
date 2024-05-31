#pragma once

#include <vector>

#include "layer.h"
#include "toki/core/core.h"
#include "toki/core/window.h"
#include "toki/renderer/renderer.h"

namespace Toki {

struct ApplicationConfig {
    WindowConfig windowConfig;
    std::filesystem::path rootDir = std::filesystem::current_path();
};

class Application {
public:
    Application(const ApplicationConfig& config);
    ~Application();

    void start();
    void stop();

    void pushLayer(Ref<Layer> layer);
    void popLayer();

    void handleEvent(Event& e);

    static Renderer& getRenderer();

private:
    Ref<Window> m_mainWindow;
    std::vector<Ref<Layer>> m_layerStack;

    bool m_running : 1 = true;

private:
    inline static Ref<Renderer> s_renderer;
};

}  // namespace Toki
