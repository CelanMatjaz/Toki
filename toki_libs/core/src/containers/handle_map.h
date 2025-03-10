#pragma once

#include "../core/types.h"
#include "../platform/platform.h"

namespace toki {

constexpr u32 INVALID_HANDLE_ID = 0;

struct Handle {
    Handle();
    Handle(u32 index): index(index), id(platform::get_time_milliseconds()) {}

    inline operator bool() {
        return id != INVALID_HANDLE_ID;
    }

    inline bool valid() {
        return this->operator bool();
    }

    u32 index{ 0 };
    u32 id{ INVALID_HANDLE_ID };
};

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
            deallocate(m_data.ptr);
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

        m_data.ptr[handle.index] = nullptr;
        --m_data.size;

        if (handle.index < m_data.next_free_index) {
            m_data.next_free_index = handle.index;
        }

        if (handle.index > m_data.last_allocated_index) {
            m_data.last_allocated_index = handle.index;
        }
    }

    void clear() {
        for (u32 i = 0; i < m_data.buffer_capacity; ++i) {
            m_data.ptr[i] = nullptr;
        }
        m_data.size = 0;
    }

    b8 contains(const Handle handle) const {
        return m_data.ptr[handle.index] != nullptr;
    }

    u32 size() const {
        return m_data.size;
    }

    void defragment([[maybe_unused]] u32 block_count) {
        TK_ASSERT(false, "Need to implement");
    }

    Handle emplace(std::initializer_list<ValueType> value) {
        HandlePtr* free_block = get_next_free();
        ValueType* ptr = free_block->ptr;
        *ptr = value;

        m_data.ptr[free_block->handle.index] = ptr;
        ++m_data.size;

        if (free_block->handle > m_data.next_free_index) {
            m_data.next_free_index = free_block->handle;
        }

        if (free_block->handle > m_data.last_allocated_index) {
            m_data.last_allocated_index = free_block->handle;
        }

        return free_block->handle;
    }

    template <typename... Args>
    Handle emplace(Args&&... args) {
        HandlePtr free_block = get_next_free();
        ValueType* ptr = new (free_block.ptr) ValueType(std::forward<Args>(args)...);
        m_data.ptr[free_block.handle.index] = ptr;
        ++m_data.size;

        if (free_block.handle.index > m_data.next_free_index) {
            m_data.next_free_index = free_block.handle;
        }

        if (free_block.handle.index > m_data.last_allocated_index) {
            m_data.last_allocated_index = free_block.handle;
        }

        return free_block.handle;
    }

    ValueType& at(const Handle handle) const {
        TK_ASSERT(contains(handle), "No value exists for provided handle");
        return ((ValueType*) m_data.values_ptr)[handle.index];
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
        return handle.index < m_data.buffer_capacity;
    }

    HandlePtr get_next_free() {
        for (u32 i = m_data.next_free_index; i < m_data.element_capacity; i++) {
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
                while (m_data->ptr[++m_index] != nullptr && m_index <= m_data->last_allocated_index)
                    ;
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

}  // namespace toki
