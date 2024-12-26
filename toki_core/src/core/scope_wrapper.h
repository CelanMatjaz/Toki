#pragma once

#include <functional>
#include <utility>

namespace toki {

template <typename T, T InvalidValue = T{}>
class Scope {
public:
    Scope(): m_value(InvalidValue), m_release(nullptr) {}
    Scope(T value, std::function<void(T)> release):
        m_value(value),
        m_release(release) {}
    ~Scope() {
        if (m_value != InvalidValue && m_release) {
            m_release(m_value);
        }
    }

    Scope(const Scope& other): Scope(other.m_value, other.m_release) {}

    Scope& operator=(const Scope& other) {
        if (this == &other) {
            return *this;
        }

        Scope temp(other);
        std::swap(m_value, other.m_value);
        std::swap(m_release, other.m_release);

        return *this;
    }

    Scope(Scope&& other):
        m_value(std::exchange(m_value, other.m_value)),
        m_release(std::exchange(m_release, other.m_release)) {}

    Scope& operator=(Scope&& other) {
        return swap(other);
    }

    Scope& swap(Scope&& other) {
        Scope temp(std::move(other));
        std::swap(m_value, other.m_value);
        std::swap(m_release, other.m_release);
        return *this;
    }

    void release() {
        m_release(m_value);
        m_release = nullptr;
        m_value = InvalidValue;
    }

    bool is_valid() const {
        return m_value != InvalidValue;
    }

    T value() {
        return m_value;
    }

    operator T() {
        return m_value;
    }

private:
    T m_value = InvalidValue;
    std::function<void(T)> m_release;
};

}  // namespace toki
