#pragma once

namespace toki {

class Renderer {
public:
    struct Config {};

public:
    Renderer() = delete;
    Renderer(const Config& config);
    virtual ~Renderer() = 0;
};

}  // namespace toki
