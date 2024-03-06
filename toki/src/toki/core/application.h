#pragma once

#include <vector>

#include "layer.h"
#include "toki/core/core.h"
#include "toki/core/window.h"
#include "toki/renderer/renderer.h"

namespace Toki {

struct ApplicationConfig {
    WindowConfig windowConfig;
};

class Application {
public:
    Application(const ApplicationConfig& config);
    ~Application();

    void start();
    void stop();

    void pushLayer(Ref<Layer> layer);
    void popLayer();

private:
    Ref<Window> m_mainWindow;
    Ref<Renderer> m_renderer;
    std::vector<Ref<Layer>> m_layerStack;

    bool m_running : 1 = true;
};

}  // namespace Toki