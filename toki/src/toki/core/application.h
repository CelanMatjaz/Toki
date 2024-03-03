#pragma once

#include <vector>

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

private:
    Ref<Window> m_mainWindow;
    Scope<Renderer> m_renderer;
};

}  // namespace Toki