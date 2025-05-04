#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../core/log.h"
#include "../memory/memory.h"
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

	static inline u64 hash(const NativeHandle handle) {
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
	HashMap(u32 element_capacity): m_data(nullptr), m_element_capacity(element_capacity), m_count(0) {
		m_data = memory_allocate_array<Bucket>(element_capacity + 1);
	}

	~HashMap() {
		memory_free(m_data);
	}

	b8 contains(const KeyType& key) const {
		return lookup_index(key) >= 0;
	}

	inline u32 capacity() const {
		return m_element_capacity;
	}

	inline u32 count() const {
		return m_count;
	}

	template <typename... Args>
	void emplace(const KeyType& key, Args&&... args) {
		u64 index = get_index(key);

		Bucket& bucket = m_data[index];
		// Empty slot
		if (bucket.psl == 0) {
			new (&bucket.value) ValueType(args...);
			bucket.key = key;
			bucket.psl = INITIAL_PSL;
			m_count++;
			return;
		}

		Bucket new_bucket{};
		new_bucket.psl = INITIAL_PSL;
		new_bucket.key = key;
		new (&new_bucket.value) ValueType(args...);

		// Handle collision
		for (u32 i = index + 1; i < m_element_capacity; i++) {
			Bucket& current_bucket = m_data[i];

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

		m_count++;
	}

	void remove(const KeyType& key) {
		i64 index = lookup_index(key);
		if (index == INVALID_INDEX) {
			return;
		}

		Bucket& bucket = m_data[index];
		while (bucket.psl != INITIAL_PSL && index < m_element_capacity - 1) {
			*bucket = m_data[index + 1];
			bucket = m_data[++index];
		}

		if (static_cast<u64>(index) < m_element_capacity) {
			m_data[index].psl = EMPTY_SLOT_PSL;
		}

		m_count--;
	}

	ValueType& operator[](const KeyType& key) const {
		return at(key);
	}

	ValueType& at(const KeyType& key) const {
		i64 index = lookup_index(key);
		return m_data[index];
	}

private:
	inline u64 get_index(const KeyType& key) const {
		return HashType::hash(static_cast<KeyType>(key)) % m_element_capacity;
	}

	i64 lookup_index(const KeyType& key) const {
		u64 index = get_index(key);
		while (m_data[index].psl != EMPTY_SLOT_PSL && index < m_element_capacity) {
			if (m_data[index].key_value.key == key) {
				return index;
			}
			++index;
		}

		TK_LOG_TRACE(
			"Attempting to lookup hash table value with a key that's not stored in the table, returning NOOP index");
		return INVALID_INDEX;
	}

	Bucket* m_data;
	u32 m_element_capacity;
	u32 m_count;

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
