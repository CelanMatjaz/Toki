#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../core/log.h"
#include "../memory/allocator.h"
#include "../string/string.h"

// NOTE(Matjaž): https://programming.guide/robin-hood-hashing.html

namespace toki {

template <typename T, typename HashType>
concept HasHashFunction = requires(T value) {
	{ HashType::hash(value) } -> SameAsConcept<u64>;
};

struct HashFunctions {
	template <typename T>
		requires(IsIntegralValue<T>)
	static u64 hash(const T v) {
		constexpr u32 multiplier = 107;

		T value = v;
		T output = 0;
		for (u32 i = 0; i < sizeof(T); i++) {
			output += value * multiplier * ((value >> i) & 1);
		}

		return value;
	}

	static inline u64 hash(const pt::Handle handle) {
		return hash(handle.handle);
	}

	static u64 hash(const char* str) {
		constexpr u32 multiplier = 107;

		u64 output = 0;
		for (u32 i = 0; i < toki::strlen(str); i++) {
			output += str[i] * multiplier;
		}

		return output;
	}
};

template <typename K, typename V, typename A = Allocator, typename H = HashFunctions>
	requires HasHashFunction<K, H> && AllocatorConcept<A>
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
	HashMap(A& allocator, u32 element_capacity):
		_allocator(allocator),
		_data(nullptr),
		_element_capacity(element_capacity),
		_count(0) {
		u32 total_size = (element_capacity + 1) * sizeof(Bucket);
		_data = reinterpret_cast<Bucket*>(allocator.allocate(total_size)) + 1;
	}

	~HashMap() {
		_allocator.free(reinterpret_cast<Bucket*>(_data) - 1);
	}

	b8 contains(const KeyType& key) const {
		return lookup_index(key) >= 0;
	}

	inline u32 capacity() const {
		return _element_capacity;
	}

	inline u32 count() const {
		return _count;
	}

	template <typename... Args>
	void emplace(const KeyType& key, Args&&... args) {
		u64 index = get_index(key);

		Bucket& bucket = _data[index];
		// Empty slot
		if (bucket.psl == 0) {
			new (&bucket.value) ValueType(args...);
			bucket.key = key;
			bucket.psl = INITIAL_PSL;
			_count++;
			return;
		}

		Bucket new_bucket{};
		new_bucket.psl = INITIAL_PSL;
		new_bucket.key = key;
		new (&new_bucket.value) ValueType(args...);

		// Handle collision
		for (u32 i = index + 1; i < _element_capacity; i++) {
			Bucket& current_bucket = _data[i];

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

		_count++;
	}

	void remove(const KeyType& key) {
		i64 index = lookup_index(key);
		if (index == INVALID_INDEX) {
			return;
		}

		Bucket& bucket = _data[index];
		while (bucket.psl != INITIAL_PSL && index < _element_capacity - 1) {
			*bucket = _data[index + 1];
			bucket = _data[++index];
		}

		if (static_cast<u64>(index) < _element_capacity) {
			_data[index].psl = EMPTY_SLOT_PSL;
		}

		_count--;
	}

	ValueType& operator[](const KeyType& key) const {
		return at(key);
	}

	ValueType& at(const KeyType& key) const {
		i64 index = lookup_index(key);
		return _data[index];
	}

private:
	inline u64 get_index(const KeyType& key) const {
		return HashType::hash(static_cast<KeyType>(key)) % _element_capacity;
	}

	i64 lookup_index(const KeyType& key) const {
		u64 index = get_index(key);
		while (_data[index].psl != EMPTY_SLOT_PSL && index < _element_capacity) {
			if (_data[index].key_value.key == key) {
				return index;
			}
			++index;
		}

		TK_LOG_TRACE(
			"Attempting to lookup hash table value with a key that's not stored in the table, returning NOOP index");
		return INVALID_INDEX;
	}

	A& _allocator;
	Bucket* _data;
	u32 _element_capacity;
	u32 _count;

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
