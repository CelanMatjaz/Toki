#pragma once

#include <utility>

#include "core/assert.h"
#include "core/base.h"
#include "core/macros.h"
#include "platform.h"

namespace toki {

namespace containers {

static_assert(std::is_same_v<Handle, uint64_t>, "Handle type must be uin64_t");

struct HandlePtr {
    Handle handle;
    void* ptr;
};

template <typename ValueType>
class HandleMap {
public:
    HandleMap() {}
    HandleMap(u32 element_count): m_data{ .element_capacity = element_count } {
        allocate();
    }

    ~HandleMap() {
        if (m_data.ptr != nullptr) {
            platform::deallocate(m_data.ptr);
        }
    }

    DELETE_COPY(HandleMap)

    HandleMap(HandleMap&& other) {
        std::swap(m_data, other.m_data);
    };

    HandleMap& operator=(HandleMap&& other) {
        std::swap(m_data, other.m_data);
        return *this;
    };

    void erase(const Handle handle) {
        if (!is_handle_in_range(handle)) {
            return;
        }

        m_data.ptr[handle] = nullptr;
        --m_data.size;

        if (handle < m_data.next_free_index) {
            m_data.next_free_index = handle;
        }

        if (handle > m_data.last_allocated_index) {
            m_data.last_allocated_index = handle;
        }
    }

    void clear() {
        for (u32 i = 0; i < m_data.buffer_capacity; ++i) {
            m_data.ptr[i] = nullptr;
        }
        m_data.size = 0;
    }

    b8 contains(const Handle handle) const {
        auto* p = (ValueType**) m_data.ptr;
        return (p)[handle] != nullptr;
    }

    u32 size() const {
        return m_data.size;
    }

    void defragment(u32 block_count) {
        TK_ASSERT(false, "Need to implement");
    }

    template <typename... Args>
    Handle emplace(Args&&... args) {
        HandlePtr free_block = get_next_free();
        ValueType* ptr = new (free_block.ptr) ValueType(std::forward<Args>(args)...);
        m_data.ptr[free_block.handle] = ptr;
        ++m_data.size;

        if (free_block.handle > m_data.next_free_index) {
            m_data.next_free_index = free_block.handle;
        }

        if (free_block.handle > m_data.last_allocated_index) {
            m_data.last_allocated_index = free_block.handle;
        }

        return free_block.handle;
    }

    ValueType& at(const Handle handle) const {
        TK_ASSERT(contains(handle), "No value exists for provided handle");
        return ((ValueType*) m_data.values_ptr)[handle];
    }

    ValueType& operator[](const Handle handle) const {
        return at(handle);
    }

    class Iterator;

    Iterator begin() {
        return Iterator(&m_data, 0);
    }

    Iterator end() {
        return Iterator(&m_data, m_data.element_capacity);
    }

private:
    void allocate() {
        TK_ASSERT(m_data.element_capacity > 0, "Cannot allocate memory for HandleMap with 0 element count");

        u32 handle_array_size = m_data.element_capacity * sizeof(ValueType*);
        m_data.buffer_capacity = m_data.element_capacity * sizeof(ValueType);

        m_data.ptr = (ValueType**) platform::allocate(handle_array_size + m_data.buffer_capacity);
        m_data.values_ptr = (ValueType*) &m_data.ptr[m_data.element_capacity];
    }

    b8 is_handle_in_range(const Handle handle) const {
        return handle < m_data.buffer_capacity;
    }

    HandlePtr get_next_free() {
        for (u32 i = m_data.next_free_index; i < m_data.buffer_capacity; i++) {
            if (m_data.ptr[i] == nullptr) {
                ++m_data.next_free_index;
                return { .handle = i, .ptr = &(m_data.values_ptr)[i] };
            }
        }

        TK_ASSERT(false, "Not enough memory allocated");
        std::unreachable();
    }

    struct InternalData {
        u32 element_capacity{};
        u32 element_size{};
        u32 buffer_capacity{};
        u32 next_free_index{};
        u32 last_allocated_index{};
        u32 size{};
        ValueType** ptr{};
        ValueType* values_ptr{};
    } m_data;

public:
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        Iterator() = delete;
        Iterator(InternalData* data, u64 index): m_data(data), m_index(index) {}
        Iterator(const Iterator& other) = default;
        Iterator& operator=(const Iterator& other) = default;

        reference operator*() const {
            return (m_data->values_ptr)[m_index];
        }

        pointer operator->() const {
            return &(m_data->values_ptr)[m_index];
        }

        Iterator& operator++() {
            ++m_index;
            auto v = m_data->ptr[m_index];

            if (auto ptr = m_data->ptr[m_index]; ptr == nullptr) {
                while (m_data->ptr[++m_index] != nullptr && m_index <= m_data->last_allocated_index);
            }

            if (m_index > m_data->last_allocated_index) {
                m_index = m_data->element_capacity;
            }

            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++m_index;
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return m_index == other.m_index;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        InternalData* m_data{};
        u64 m_index{};
    };
};

}  // namespace containers

}  // namespace toki
