#pragma once

#include <chrono>

namespace Toki {

class Timer {
public:
    Timer();
    Timer(const char* name);
    ~Timer();

private:
    const char* m_name = nullptr;
    std::chrono::steady_clock::time_point m_start;
};

}  // namespace Toki
