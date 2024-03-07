#include "timer.h"

#include <print>

namespace Toki {

Timer::Timer() : Timer("timer") {}

Timer::Timer(const char* name) : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}

Timer::~Timer() {
    using namespace std::chrono;
    std::println("Timer {} ran for {}", m_name, duration_cast<nanoseconds>(high_resolution_clock::now() - m_start));
}

}  // namespace Toki
