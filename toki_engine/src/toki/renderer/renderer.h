#pragma once

#include "core/id.h"
#include "engine/window.h"
#include "renderer/configs.h"
#include "renderer/renderer_api.h"

namespace toki {

class engine;
struct renderer_context;

class renderer {
    friend engine;

public:
    struct Config {
        std::shared_ptr<window> initialWindow;
    };

private:
    static std::shared_ptr<renderer> create(const Config& config);

public:
    renderer() = delete;
    renderer(const Config& config);
    ~renderer() = default;

    DELETE_COPY(renderer);
    DELETE_MOVE(renderer);

    std::shared_ptr<renderer_api> get_renderer_api() const;

public:
    virtual handle create_shader(const shader_create_config& config) = 0;
    virtual void destroy_shader(handle handle) = 0;

    virtual handle create_buffer(const buffer_create_config& config) = 0;
    virtual void destroy_buffer(handle handle) = 0;

protected:
    virtual b8 begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void present() = 0;

    void add_window(std::shared_ptr<window> window);
    std::shared_ptr<renderer_context> _context{};
    std::shared_ptr<renderer_api> _renderer_api{};
};

}  // namespace toki
