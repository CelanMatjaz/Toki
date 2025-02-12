#pragma once

#include "memory/allocators/dynamic_allocator.h"

namespace toki {

template <typename T>
class BasicRef {
public:
    BasicRef() = default;
    BasicRef(DynamicAllocator* allocator, u64 size = 1):
        m_allocator(allocator),
        m_size(size * sizeof(T) + sizeof(u64)) {
        m_ptr = allocator->allocate_aligned(size * sizeof(T) + sizeof(u64), alignof(u64));
        get_counter() = 1;
    }

    ~BasicRef() {
        if (get_counter() == 1) {
            m_allocator->free(m_ptr);
        } else {
            get_counter()--;
        }
    };

    BasicRef(const BasicRef<T>& other) {
        m_allocator = other.m_allocator;
        m_ptr = other.m_ptr;
        m_size = other.m_size;

        get_counter() += 1;
    };

    BasicRef<T>& operator=(const BasicRef<T>& other) {
        m_allocator = other.m_allocator;
        m_ptr = other.m_ptr;
        m_size = other.m_size;

        get_counter() += 1;

        return *this;
    };

    BasicRef(BasicRef<T>&& other) {
        if (this != &other) {
            m_allocator = other.m_allocator;
            m_ptr = other.m_ptr;
            m_size = other.m_size;

            get_counter()++;
        }
    }

    BasicRef<T>& operator=(BasicRef<T>&& other) {
        m_allocator = other.m_allocator;
        m_ptr = other.m_ptr;
        m_size = other.m_size;

        get_counter()++;

        return *this;
    }

    T* operator->() const {
        return get();
    }

    T* get() const {
        return reinterpret_cast<T*>(reinterpret_cast<u64*>(m_ptr) + 1);
    }

    operator T*() const {
        return get();
    }

    u64 count() const {
        return (m_size - sizeof(u64)) / sizeof(T);
    }

    u64 capacity() const {
        return m_size - sizeof(u64);
    }

    T& operator[](const u64 index) const {
        TK_ASSERT(index < count(), "Invalid index");
        return get()[index];
    }

    void reset() {
        m_allocator->free(m_ptr);
        m_ptr = nullptr;
        m_size = 0;
    }

    operator bool() {
        return m_ptr != nullptr;
    }

private:
    void swap(BasicRef<T>&& other) {
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_size, other.m_size);
    }

    u64& get_counter() {
        return *reinterpret_cast<u64*>(m_ptr);
    }

    DynamicAllocator* m_allocator{};
    void* m_ptr;
    u64 m_size;
};

}  // namespace toki
