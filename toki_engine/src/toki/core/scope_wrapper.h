#pragma once

#include <functional>
#include <utility>

namespace toki {

template <typename T, T InvalidValue = T{}>
class Scoped {
public:
    Scoped(): m_value(InvalidValue), m_release(nullptr) {}
    Scoped(T value, std::function<void(T)> release): m_value(value), m_release(release) {}
    ~Scoped() {
        if (m_value != InvalidValue && m_release) {
            m_release(m_value);
        }
    }

    Scoped(const Scoped& other): Scoped(other.m_value, other.m_release) {}

    Scoped& operator=(const Scoped& other) {
        if (this == &other) {
            return *this;
        }

        Scoped temp(other);
        std::swap(m_value, other.m_value);
        std::swap(m_release, other.m_release);

        return *this;
    }

    Scoped(Scoped&& other): m_value(std::exchange(m_value, other.m_value)), m_release(std::exchange(m_release, other.m_release)) {}

    Scoped& operator=(Scoped&& other) {
        return swap(other);
    }

    Scoped& swap(Scoped& other) {
        Scoped temp(std::move(other));
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

    T& ref() {
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
