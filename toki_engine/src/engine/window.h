#pragma once

#include <string>

#include "../core/core.h"

namespace toki {

class Engine;

class Window {
public:
    struct Config {
        i32 width = 0;
        i32 height = 0;
        std::string title;
    };

public:
    static std::shared_ptr<Window> create(const Config& config);
    Window() = delete;
    Window(const Config& config);
    ~Window();

    const void* get_handle() const;
    bool should_close() const;

private:
    friend Engine;

    static void poll_events();

    void* m_handle{};
};

}  // namespace toki
