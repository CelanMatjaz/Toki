#pragma once

#include <functional>
#include <string>

#include "core/assert.h"
#include "core/base.h"
#include "memory/allocators/stack_allocator.h"

namespace toki {

template <typename T, typename HashType = T>
concept HasHashFunction = requires(const T& value) {
    { HashType::hash(value) } -> std::same_as<u64>;
};

struct StringHash {
    static u64 hash(const std::string& value) {
        return std::hash<std::string>{}(value);
    }
};

struct IntHash {
    static u64 hash(const int& value) {
        return std::hash<int>{}(value);
    }
};

template <
    typename KeyType,
    typename ValueType,
    typename HashType = KeyType,
    typename KeyEqual = std::equal_to<KeyType>,
    ValueType InvalidValue = ValueType{}>
    requires HasHashFunction<KeyType, HashType>
class HashMap {
public:
    struct Pair {
        KeyType key;
        ValueType value;
    };

    struct BucketEntry {
        BucketEntry* next;
        Pair pair;
    };

    HashMap(u32 element_capacity, StackAllocator* allocator): m_data{ element_capacity, 0 } {
        m_data.first = m_data.ptr =
            (BucketEntry*) allocator->allocate_aligned(sizeof(BucketEntry) * (m_data.capacity), alignof(BucketEntry));
    }
    ~HashMap() {}

    u32 size() const {
        return m_data.size;
    }

    template <typename... Args>
    void emplace(const KeyType& key, Args&&... args) {
        TK_ASSERT(m_data.size < m_data.capacity, "HashMap capacity full ({})", m_data.capacity);

        u64 hash = HashType::hash(key);
        u32 index = hash % m_data.capacity;

        check_for_clash(index);

        BucketEntry* ptr =
            new (&m_data.ptr[index]) BucketEntry{ nullptr, { key, { ValueType(std::forward<Args>(args)...) } } };
        ++m_data.size;

        if (ptr < m_data.first) {
            ptr->next = m_data.first;
            m_data.first = ptr;
        } else {
            BucketEntry* p = m_data.first;
            while (p->next < ptr && p->next != nullptr) {
                p = p->next;
            }

            ptr->next = p->next;
            p->next = ptr;
        }

        if (index < m_data.first_index || m_data.first_index == -1) {
            m_data.first_index = index;
        }
    }

    ValueType& at(const KeyType& key) const {
        return m_data.ptr[get_index(key)];
    }

    ValueType& operator[](const KeyType& key) const {
        return at(key);
    }

    b8 contains(const KeyType& key) const {
        u64 hash = HashType::hash(key);
        u32 index = hash % m_data.capacity;

        return m_data.ptr[index].pair.value != InvalidValue;
    }

    class Iterator;

    Iterator begin() {
        return Iterator(&m_data, m_data.first);
    }

    Iterator end() {
        return Iterator(&m_data, nullptr);
    }

private:
    u64 get_index(const KeyType& key) {
        return HashType::hash(key) % m_data.capacity;
    }

    b8 check_for_clash(u64 index) {
        TK_ASSERT(m_data.ptr[index].pair.value == InvalidValue, "No key clash logic implemented");
        return false;
    }

    struct InternalData {
        u32 capacity;
        u32 size;
        i32 first_index = -1;
        BucketEntry* ptr = nullptr;
        BucketEntry* first = nullptr;
    } m_data;

public:
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Pair;
        using difference_type = std::ptrdiff_t;
        using pointer = Pair*;
        using reference = Pair&;

        Iterator() = delete;
        Iterator(InternalData* data, BucketEntry* p): m_data(data), m_entry(p) {}
        Iterator(const Iterator& other) = default;
        Iterator& operator=(const Iterator& other) = default;

        reference operator*() const {
            return m_entry->pair;
        }

        pointer operator->() const {
            return &m_entry->pair;
        }

        Iterator& operator++() {
            m_entry = m_entry->next;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            m_entry = m_entry->next;
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return m_entry == other.m_entry;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        InternalData* m_data;
        BucketEntry* m_entry{};
    };

private:
};

}  // namespace toki
