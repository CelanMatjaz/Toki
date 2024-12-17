#pragma once

namespace toki {

class Window {
public:
    struct Config {};

public:
    Window() = delete;
    Window(const Config& config);
    ~Window();

    static void pollEvents();
};

}  // namespace toki
