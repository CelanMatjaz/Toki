#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include "core/base.h"

namespace toki {

template <typename Key, typename Value>
class array_map {
public:
    array_map() = default;
    ~array_map() = default;

    void add(const Key& key, const Value& value) {
        _array.push_back(value);
        _map.emplace(key, _array.size() - 1);
    }

    template <typename... Args>
    void emplace(Key key, Args&&... args) {
        _array.emplace_back(std::forward<Args>(args)...);
        _map.emplace(key, _array.size() - 1);
    }

    void emplace(Key key, const Value& value) {
        _array.emplace_back(std::forward(value));
        _map.emplace(key, _array.size() - 1);
    }

    Value& get(const Key& key) {
        return _array.at(_map.at(key));
    }

    void remove(const Key& key) {
        TK_ASSERT(_array.size() > 0, "Internal array size should not be 0");
        u32 current_index = _map[key];
        std::swap(_array[current_index], _array.back());
        _array.resize(_array.size() - 1);
        _map.erase(key);
    }

    void clear() {
        _array.clear();
        _map.clear();
    }

    bool contains(const Key& key) const {
        return _map.contains(key);
    }

    u32 size() const {
        return _map.size();
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

        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    Iterator begin() {
        return Iterator(_array.data());
    }

    Iterator end() {
        return Iterator(_array.data() + _array.size());
    }

private:
    std::unordered_map<Key, u32> _map;
    std::vector<Value> _array;
};

}  // namespace toki
