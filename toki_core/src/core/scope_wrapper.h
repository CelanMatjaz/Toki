#pragma once

#include <functional>

namespace toki {

template <typename T, T InvalidValue = T{}>
class Scope {
public:
    inline Scope(): m_value(InvalidValue) {}
    inline Scope(T value, std::function<void(T)> release):
        m_value(value),
        m_release(release) {}
    inline ~Scope() {
        m_release(m_value);
    }

    inline Scope(const Scope& other): Scope(other.m_value, other.m_release) {}

    inline Scope& operator=(const Scope& other) {
        if (this == &other) {
            return *this;
        }

        Scope temp(other);
        std::swap(m_value, other.m_value);
        std::swap(m_release, other.m_release);

        return *this;
    }

    inline Scope(Scope&& other):
        m_value(std::exchange(m_value, other.m_value)),
        m_release(std::exchange(m_release, other.m_release)) {}

    inline Scope& operator=(Scope&& other) {
        return swap(other);
    }

    inline Scope& swap(Scope&& other) {
        Scope temp(std::move(other));
        std::swap(m_value, other.m_value);
        std::swap(m_release, other.m_release);
        return *this;
    }

    inline bool is_valid() const {
        return m_value != InvalidValue;
    }

    inline T value() const {
        return m_value;
    }

private:
    T m_value = InvalidValue;
    std::function<void(T)> m_release;
};

}  // namespace toki
