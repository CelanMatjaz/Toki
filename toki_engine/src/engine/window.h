#pragma once

#include <toki/core.h>

#include <string>

namespace toki {

class Window {
public:
    struct Config {
        i32 width = 0;
        i32 height = 0;
        std::string title;
    };

public:
    Window() = delete;
    Window(const Config& config);
    ~Window();

    bool should_close() const;

    static void poll_events();

private:
    void* m_handle{};
};

}  // namespace toki
