#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include "core/base.h"

namespace toki {

template <typename Key, typename Value>
class ArrayMap {
public:
    ArrayMap() = default;
    ~ArrayMap() = default;

    void add(const Key& key, const Value& value) {
        m_array.push_back(value);
        m_map.emplace(key, m_array.size() - 1);
    }

    template <typename... Args>
    void emplace(Key key, Args&&... args) {
        m_array.emplace_back(std::forward<Args>(args)...);
        m_map.emplace(key, m_array.size() - 1);
    }

    void emplace(Key key, const Value& value) {
        m_array.emplace_back(std::forward(value));
        m_map.emplace(key, m_array.size() - 1);
    }

    Value& get(const Key& key) {
        return m_array.at(m_map.at(key));
    }

    // TODO: switch map values too
    void erase(const Key& key) {
        TK_ASSERT(m_array.size() > 0, "Internal array size should not be 0");
        u32 current_index = m_map[key];
        std::swap(m_array[current_index], m_array.back());
        m_array.resize(m_array.size() - 1);
        m_map.erase(key);
    }

    void clear() {
        m_array.clear();
        m_map.clear();
    }

    b8 contains(const Key& key) const {
        return m_map.contains(key);
    }

    u32 size() const {
        return m_map.size();
    }

    Value& operator[](const Key& key) {
        return get(key);
    }

    class Iterator {
    private:
        Value* current;

    public:
        explicit Iterator(Value* ptr): current(ptr) {}

        Value& operator*() const {
            return *current;
        }

        Value* operator->() const {
            return current;
        }

        Iterator& operator++() {
            ++current;
            return *this;
        }

        b8 operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    Iterator begin() {
        return Iterator(m_array.data());
    }

    Iterator end() {
        return Iterator(m_array.data() + m_array.size());
    }

private:
    std::unordered_map<Key, u32> m_map;
    std::vector<Value> m_array;
};

}  // namespace toki
