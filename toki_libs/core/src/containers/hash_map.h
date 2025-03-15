#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../core/logging.h"

// NOTE(Matjaž): https://programming.guide/robin-hood-hashing.html

namespace toki {

template <typename T, typename HashType>
concept HasHashFunction = requires(const T& value) {
    { HashType::hash(value) } -> SameAsConcept<u64>;
};

template <typename K, typename V, typename H = K>
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
    HashMap(AllocatorConcept auto& allocator, u64 element_capacity):
        mData(nullptr),
        mElementCapacity(element_capacity) {
        u32 total_size = (element_capacity + 1) * sizeof(Bucket);
        mData = reinterpret_cast<Bucket*>(allocator.allocate(total_size)) + 1;
    }
    ~HashMap() {}

    Bucket* mData;
    u64 mElementCapacity;

    b8 contains(const KeyType& key) const {
        return lookup_index(key) >= 0;
    }

    inline u32 capacity() const {
        return mElementCapacity;
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
    }

    void remove(const KeyType& key) const {
        i64 index = lookup_index(key);
        if (index == INVALID_INDEX) {
            return;
        }

        Bucket& bucket = mData[index];
        while (bucket.psl != INITIAL_PSL) {
            *bucket = mData[index + 1];
            bucket = mData[++index];
        }
    }

    ValueType& operator[](const KeyType& key) const {
        return at(key);
    }

    ValueType& at(const KeyType& key) const {
        i64 index = lookup_index(key);
        return mData[index];
    }

private:
    inline u64 get_index(const KeyType& key) {
        return HashType::hash(key) % mElementCapacity;
    }

    i64 lookup_index(const KeyType& key) {
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
};

}  // namespace toki
