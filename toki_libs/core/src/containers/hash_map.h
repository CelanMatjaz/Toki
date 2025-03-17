#pragma once

#include "../core/common.h"
#include "../core/logging.h"
#include "../platform/platform.h"

// NOTE(Matjaž): https://programming.guide/robin-hood-hashing.html

namespace toki {

template <typename T, typename HashType>
concept HasHashFunction = requires(const T& value) {
    { HashType::hash(value) } -> SameAsConcept<u64>;
};

struct HashFunctions {
    // NOTE(Matjaž): Should this be changed?
    // Arbitrary integral hashing function
    template <typename T>
        requires(IsIntegral<T>)
    u64 hash(T& v) {
        constexpr u32 multiplier = 107;

        T value = v;
        T output = 0;
        for (u32 i = 0; i < sizeof(T); i++) {
            output += value * multiplier * ((value >> i) & 1);
        }

        return value;
    }

    inline u64 hash(NativeWindowHandle& value) {
        return hash(value.i64);
    }
};

template <typename K, typename V, typename H = HashFunctions>
    requires HasHashFunction<K, H>
class HashMap {
private:
    using KeyType = K;
    using ValueType = V;
    using HashType = H;
    constexpr static u64 EMPTY_SLOT_PSL = 0;
    constexpr static u64 INITIAL_PSL = 1;

    // NOTE(Matjaž): invalid index is -1 to return
    // the NOOP value to not crash program
    constexpr static i32 INVALID_INDEX = -1;

    struct Bucket {
        u64 psl;
        KeyType key;
        ValueType value;
    };

public:
    HashMap(AllocatorConcept auto& allocator, u32 element_capacity):
        mData(nullptr),
        mElementCapacity(element_capacity),
        mCount(0) {
        u32 total_size = (element_capacity + 1) * sizeof(Bucket);
        mData = reinterpret_cast<Bucket*>(allocator.allocate(total_size)) + 1;
    }

    HashMap(void* buffer, u64 element_capacity_plus_1):
        mData(reinterpret_cast<Bucket*>(buffer) + 1),
        mElementCapacity(element_capacity_plus_1),
        mCount(0) {}

    ~HashMap() {}

    b8 contains(const KeyType& key) const {
        return lookup_index(key) >= 0;
    }

    inline u32 capacity() const {
        return mElementCapacity;
    }

    inline u32 count() const {
        return mCount;
    }

    template <typename... Args>
    void emplace(const KeyType& key, Args&&... args) {
        u64 index = get_index(key);

        Bucket& bucket = mData[index];
        // Empty slot
        if (bucket.psl == 0) {
            new (&bucket.value) ValueType(args...);
            bucket.key = key;
            bucket.psl = INITIAL_PSL;
            mCount++;
            return;
        }

        Bucket new_bucket{};
        new_bucket.psl = INITIAL_PSL;
        new_bucket.key = key;
        new (&new_bucket.value) ValueType(args...);

        // Handle collision
        for (u32 i = index + 1; i < mElementCapacity; i++) {
            Bucket& current_bucket = mData[i];

            // Found empty slot
            if (current_bucket.psl == 0) {
                current_bucket = new_bucket;
                return;
            }
            // Found slot with lower PSL
            else if (current_bucket.psl < new_bucket.psl) {
                swap(new_bucket, current_bucket);
            }

            ++new_bucket.psl;
        }

        mCount++;
    }

    void remove(const KeyType& key) {
        i64 index = lookup_index(key);
        if (index == INVALID_INDEX) {
            return;
        }

        Bucket& bucket = mData[index];
        while (bucket.psl != INITIAL_PSL && index < mElementCapacity - 1) {
            *bucket = mData[index + 1];
            bucket = mData[++index];
        }

        if (static_cast<u64>(index) < mElementCapacity) {
            mData[index].psl = EMPTY_SLOT_PSL;
        }

        mCount--;
    }

    ValueType& operator[](const KeyType& key) const {
        return at(key);
    }

    ValueType& at(const KeyType& key) const {
        i64 index = lookup_index(key);
        return mData[index];
    }

private:
    inline u64 get_index(const KeyType& key) const {
        return HashType::hash(static_cast<KeyType>(key)) % mElementCapacity;
    }

    i64 lookup_index(const KeyType& key) const {
        u64 index = get_index(key);
        while (mData[index].psl != EMPTY_SLOT_PSL && index < mElementCapacity) {
            if (mData[index].key_value.key == key) {
                return index;
            }
            ++index;
        }

        TK_LOG_TRACE(
            "Attempting to lookup hash table value with a key that's not stored in the table, returning NOOP index");
        return INVALID_INDEX;
    }

    Bucket* mData;
    u32 mElementCapacity;
    u32 mCount;

    // public:
    //     class Iterator;
    //
    //     Iterator begin() {
    //         return Iterator(mData, 0);
    //     }
    //
    //     Iterator end() {
    //         return Iterator(mData, mElementCapacity);
    //     }
    //
    //     class Iterator {
    //     public:
    //         Iterator() = delete;
    //         Iterator(Bucket* data, u64 index): mData(data), mIndex(index) {}
    //         Iterator(const Iterator& other) = default;
    //         Iterator& operator=(const Iterator& other) = default;
    //
    //         ValueType& operator*() const {
    //             return mData[mIndex].value;
    //         }
    //
    //         ValueType* operator->() const {
    //             return &mData[mIndex].value;
    //         }
    //
    //         Iterator& operator++() {
    //             ++mIndex;
    //
    //             while (mData[mIndex].psl == EMPTY_SLOT_PSL) {
    //                 ++mIndex;
    //             }
    //
    //             return *this;
    //         }
    //
    //         Iterator operator++(int) {
    //             Iterator temp = *this;
    //             ++mIndex;
    //             return temp;
    //         }
    //
    //         bool operator==(const Iterator& other) const {
    //             return mIndex == other.mIndex;
    //         }
    //
    //         bool operator!=(const Iterator& other) const {
    //             return !(*this == other);
    //         }
    //
    //     private:
    //         Bucket* mData{};
    //         u64 mIndex{};
    //     };
};

}  // namespace toki
