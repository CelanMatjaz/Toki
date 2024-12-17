#pragma once

namespace toki {

class Application {
public:
    struct Config {};

public:
    Application() = delete;
    Application(const Config& config);
    ~Application();

private:
    void run();
};

}  // namespace toki
