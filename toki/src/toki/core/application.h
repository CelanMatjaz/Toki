#pragma once

#include <vector>

#include "core.h"
#include "window.h"

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
    Scope<Window> m_mainWindow;
};

}  // namespace Toki