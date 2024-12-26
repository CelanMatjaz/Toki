#pragma once

#include <toki/core.h>

#include "renderer_state.h"

namespace toki {

class Renderer {
public:
    struct Config {};

public:
    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer();

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

public:  // API
private:
    RendererState m_state;
};

}  // namespace toki
