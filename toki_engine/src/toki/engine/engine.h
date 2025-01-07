#pragma once

#include <memory>

#include "engine/view.h"
#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class engine {
public:
    struct config {
        ref<window> initialWindow;
    };

public:
    engine() = delete;
    engine(const config& config);
    ~engine();

    DELETE_COPY(engine);
    DELETE_MOVE(engine);

    void run();

    void add_view(ref<view> view);

private:
    bool m_isRunning = false;

    std::vector<ref<window>> _windows;
    std::vector<ref<view>> _views;
    ref<renderer> _renderer;
};

}  // namespace toki
