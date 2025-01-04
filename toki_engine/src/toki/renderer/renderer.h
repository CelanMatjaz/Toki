#pragma once

#include "engine/window.h"

namespace toki {

class Renderer {

public:
    struct Config {
        std::shared_ptr<Window> initialWindow;
    };

public:
    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer();

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

public:  // API
private:
    void add_window(std::shared_ptr<Window> window);
};

}  // namespace toki
