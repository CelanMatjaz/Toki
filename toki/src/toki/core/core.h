#pragma once

#include <memory>
#include <string>

namespace Toki {

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> createScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> createRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
class WrappedRef {
public:
    WrappedRef(T* ref) : m_ref(ref){};
    WrappedRef(const T* ref) : m_ref(ref){};
    ~WrappedRef() = default;
    WrappedRef(const WrappedRef<T>& other) : m_ref(other.m_ref) {}
    WrappedRef(WrappedRef<T>&& other) : m_ref(other.m_ref) {}
    WrappedRef<T>& operator=(const WrappedRef<T>& other) { return *this = WrappedRef<T>(other); }
    WrappedRef<T>& operator=(WrappedRef<T>&& other) {
        m_ref = other.m_ref;
        return *this;
    }

    const T* operator->() const { return m_ref; }
    const T* get() const { return m_ref; }

private:
    const T* m_ref;
};
template <typename T>
constexpr WrappedRef<T> createWrappedRef(T* ref) {
    return ref;
}
template <typename T>
constexpr WrappedRef<T> createWrappedRef(const T* ref) {
    return ref;
}

}  // namespace Toki
