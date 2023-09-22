#include "tkpch.h"
#include "window.h"
#include "toki/core/engine.h"
#include "platform/glfw/window.h"

#ifdef TK_WIN32
#include "platform/windows/windows_window.h"
#else 
#include "platform/glfw/window.h"
#endif

namespace Toki {

    Ref<Window> Window::create(const WindowConfig& windowConfig, void* engine) {
#ifdef TK_WIN32
        return createRef<TokiWindowsWindow>(windowConfig, engine);
#else 
        return createRef<TokiWindow>(windowConfig, engine);
#endif
    }

    Window::Window(const WindowConfig& windowConfig, void* engine) : windowConfig(windowConfig), engine(engine) { }

}