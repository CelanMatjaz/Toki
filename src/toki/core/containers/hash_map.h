#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/log.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/types.h>
#include <toki/core/utils/memory.h>

// NOTE(Matjaž): https://programming.guide/robin-hood-hashing.html

namespace toki {

template <typename T, typename HashType>
concept HasHashFunction = requires(T value) {
	{ HashType::hash(value) } -> CIsSame<u64>;
};

struct HashFunctions {
	template <typename T>
		requires(CIsIntegral<T>)
	static u64 hash(const T v) {
		static constexpr u32 multiplier = 107;

		T value	 = v;
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
		static constexpr u32 multiplier = 107;

		u64 output = 0;
		for (u32 i = 0; i < toki::strlen(str); i++) {
			output += str[i] * multiplier;
		}

		return output;
	}

	static u64 hash(toki::StringView str) {
		return hash(str.data());
	}
};

template <typename K, typename V, typename H = HashFunctions, CIsAllocator AllocatorType = DefaultAllocator>
	requires HasHashFunction<K, H>
class HashMap {
private:
	using KeyType						= K;
	using ValueType						= V;
	using HashType						= H;
	constexpr static u64 EMPTY_SLOT_PSL = 0;
	constexpr static u64 INITIAL_PSL	= 1;

	// NOTE(Matjaž): invalid index is -1 to return
	// the NOOP value to not crash program
	constexpr static i64 INVALID_INDEX = static_cast<i64>(-1);

	struct Bucket {
		u64 psl;
		KeyType key;
		ValueType value;
	};

public:
	HashMap() = default;

	HashMap(u32 element_capacity) {
		reset(element_capacity);
	}

	~HashMap() {
		AllocatorType::free(m_data);
	}

	HashMap(HashMap&& other) {
		m_data			  = other.m_data;
		m_count			  = other.m_count;
		m_elementCapacity = other.m_elementCapacity;

		other.m_data			= {};
		other.m_count			= {};
		other.m_elementCapacity = {};
	}

	HashMap& operator=(HashMap&& other) {
		m_data			  = other.m_data;
		m_count			  = other.m_count;
		m_elementCapacity = other.m_elementCapacity;

		other.m_data			= {};
		other.m_count			= {};
		other.m_elementCapacity = {};

		return *this;
	}

	void reset(u32 element_count) {
		m_elementCapacity = element_count;
		m_count			  = 0;
		m_data			  = reinterpret_cast<Bucket*>(
			   AllocatorType::reallocate(m_data, static_cast<u64>(element_count * sizeof(Bucket))));
		toki::memset(m_data, {}, element_count * sizeof(Bucket));
	}

	b8 contains(const KeyType& key) const {
		return lookup_index(key) >= 0;
	}

	inline u32 capacity() const {
		return m_elementCapacity;
	}

	inline u32 count() const {
		return m_count;
	}

	template <typename... Args>
	void emplace(const KeyType& key, Args&&... args) {
		TK_ASSERT(m_elementCapacity > 0);
		u64 index = get_index(key);

		Bucket& bucket = m_data[index];
		// Empty slot
		if (bucket.psl == 0) {
			construct_at(&bucket.value, toki::forward<Args>(args)...);
			bucket.key = key;
			bucket.psl = INITIAL_PSL;
			m_count++;
			return;
		}

		Bucket new_bucket{};
		new_bucket.psl = INITIAL_PSL;
		new_bucket.key = key;
		construct_at(&new_bucket.value, toki::forward<Args>(args)...);

		// Handle collision
		for (u32 i = index + 1; i < m_elementCapacity; i++) {
			Bucket& current_bucket = m_data[i];

			// Found empty slot
			if (current_bucket.psl == 0) {
				current_bucket = toki::move(new_bucket);
				return;
			}
			// Found slot with lower PSL
			else if (current_bucket.psl < new_bucket.psl) {
				Bucket temp_bucket = toki::move(current_bucket);
				current_bucket	   = toki::move(new_bucket);
				new_bucket		   = toki::move(temp_bucket);
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
		while (bucket.psl != INITIAL_PSL && index < m_elementCapacity - 1) {
			*bucket = m_data[index + 1];
			bucket	= m_data[++index];
		}

		if (static_cast<u64>(index) < m_elementCapacity) {
			m_data[index].psl = EMPTY_SLOT_PSL;
		}

		m_count--;
	}

	ValueType& operator[](const KeyType& key) const {
		return at(key);
	}

	ValueType& at(const KeyType& key) const {
		i64 index = lookup_index(key);
		return m_data[index].value;
	}

private:
	inline u64 get_index(const KeyType& key) const {
		TK_ASSERT(m_elementCapacity > 0);
		return HashType::hash(static_cast<KeyType>(key)) % m_elementCapacity;
	}

	i64 lookup_index(const KeyType& key) const {
		u64 index = get_index(key);
		while (m_data[index].psl != EMPTY_SLOT_PSL && index < m_elementCapacity) {
			if (m_data[index].key == key) {
				return index;
			}
			++index;
		}

		TK_LOG_WARN(
			"Attempting to lookup hash table value with a key that's not stored in the table, returning NOOP index");
		return INVALID_INDEX;
	}

	Bucket* m_data{};
	u32 m_elementCapacity{};
	u32 m_count{};
};

}  // namespace toki
