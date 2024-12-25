#pragma once

#include <toki/core.h>

#include <memory>

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

    struct RendererState;

public:  // API
private:
    std::unique_ptr<RendererState> m_state;
};

}  // namespace toki
