#include "tkpch.h"
#include "window.h"
#include "toki/core/engine.h"
#include "platform/glfw/window.h"

namespace Toki {

    Ref<Window> Window::create(const WindowConfig& windowConfig, Engine* engine) {
        return createRef<TokiWindow>(windowConfig, engine);
    }

    Window::Window(const WindowConfig& windowConfig, Engine* engine) : engine(engine) {
        width = windowConfig.width;
        height = windowConfig.height;
    }

}