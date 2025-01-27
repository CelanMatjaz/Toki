#pragma once

#include "memory/allocators/basic_allocator.h"

namespace toki {

template <typename T>
class BasicRef {
public:
    BasicRef() = default;
    BasicRef(BasicAllocator* allocator, u64 size = 1): m_allocator(allocator), m_size(size * sizeof(T) + sizeof(u64)) {
        m_handle = allocator->allocate_aligned(size * sizeof(T) + sizeof(u64), alignof(u64));
        get_counter() = 1;
    }

    ~BasicRef() {
        if (m_handle == INVALID_ALLOCATOR_HANDLE) {
            return;
        }

        if (get_counter() == 1) {
            m_allocator->free(m_handle);
        } else {
            get_counter()--;
        }
    };

    BasicRef(const BasicRef& other) {
        m_allocator = other.m_allocator;
        m_handle = other.m_handle;
        m_size = other.m_size;

        get_counter() += 1;
    };

    BasicRef& operator=(const BasicRef& other) {
        m_allocator = other.m_allocator;
        m_handle = other.m_handle;
        m_size = other.m_size;

        get_counter() += 1;

        return *this;
    };

    BasicRef(BasicRef&& other) {
        if (this != &other) {
            swap(other);
        }
    }

    BasicRef<T>& operator=(BasicRef<T>&& other) {
        m_allocator = other.m_allocator;
        m_handle = other.m_handle;
        m_size = other.m_size;

        other.m_allocator = nullptr;
        other.m_handle = {};
        other.m_size = 0;

        return *this;
    }

    T* operator->() const {
        return get();
    }

    T* get() const {
        u64* a = m_allocator->get<u64>(m_handle);
        T* b = m_allocator->get<T>(m_handle);

        return reinterpret_cast<T*>(reinterpret_cast<u64*>(m_allocator->get<u64>(m_handle)) + 1);
    }

    u64 size() const {
        return m_size / sizeof(T);
    }

    u64 buffer_size() const {
        return m_size;
    }

    T& operator[](const u32 index) const {
        TK_ASSERT(index < size(), "Invalid index");
        return get()[index];
    }

    void reset() {
        m_allocator->free(m_handle);
        m_handle = nullptr;
        m_size = 0;
    }

    operator bool() {
        return m_handle != INVALID_ALLOCATOR_HANDLE;
    }

private:
    void swap(BasicRef<T>&& other) {
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_handle, other.m_handle);
        std::swap(m_size, other.m_size);
    }

    u64& get_counter() {
        return *reinterpret_cast<u64*>(m_allocator->get(m_handle));
    }

    BasicAllocator* m_allocator{};
    u64 m_handle{ INVALID_ALLOCATOR_HANDLE };
    u64 m_size{};
};

}  // namespace toki
