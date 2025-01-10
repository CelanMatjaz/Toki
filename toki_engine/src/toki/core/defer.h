#pragma once

#include <functional>

namespace toki {

class Defer {
public:
    Defer() = delete;
    Defer(std::function<void()> defer_fn): m_deferFn(defer_fn) {}
    ~Defer() {
        if (m_deferFn) {
            m_deferFn();
        }
    }

private:
    std::function<void()> m_deferFn;
};

}  // namespace toki
