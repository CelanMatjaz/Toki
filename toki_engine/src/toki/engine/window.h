#pragma once

#include <memory>
#include <string>

#include "core/core.h"

namespace toki {

class Window {
public:
    struct Config {
        i32 width = 0;
        i32 height = 0;
        std::string title;
    };

public:
    static Ref<Window> create(const Config& config);

    Window() = delete;
    Window(const Config& config);
    virtual ~Window() = 0;

    virtual bool should_close() const = 0;
    virtual void* get_handle() const = 0;
    static void poll_events();
};

}  // namespace toki
