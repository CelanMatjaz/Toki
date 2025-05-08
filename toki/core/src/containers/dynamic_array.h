#pragma once

#include "../core/assert.h"
#include "../memory/memory.h"

namespace toki {

// DynamicArray
//
// Class is used for dynamic allocations with different types
// of allocators that match the AllocatorConcept concept.

template <typename T>
class DynamicArray {
public:
	DynamicArray() {}
	DynamicArray(u64 count): m_capacity(count), m_data(memory_allocate(sizeof(T) * count)) {}

	DynamicArray(u64 size, T&& default_value): DynamicArray(size) {
		for (u32 i = 0; i < size; i++) {
			m_data[i] = default_value;
		}
	}

	DynamicArray(DynamicArray&& other): m_data(other.m_data), m_capacity(other.m_capacity) {
		other.m_data = nullptr;
		other.m_capacity = 0;
	}

	~DynamicArray() {
		if (m_data != nullptr) {
			memory_free(m_data);
		}
	}

	DELETE_COPY(DynamicArray);

	DynamicArray& operator=(DynamicArray&& other) {
		if (this != &other) {
			swap(move(other));
		}
		return *this;
	}

	// This function will NOT resize/reallocate a new
	// buffer if new_size is less than mSize.
	void resize(u32 new_capacity) {
		if (new_capacity > m_capacity) {
			m_data = memory_reallocate_array<T>(m_data, new_capacity);
		}
		m_capacity = new_capacity;
	}

	inline void shrink_to_count(u32 new_size) {
		TK_ASSERT(new_size <= m_capacity, "New size cannot be larger than old size when shrinking");
		m_capacity = new_size;
	}

	inline T& operator[](u64 index) const {
		return m_data[index];
	}

	inline T* data() const {
		return m_data;
	}

	inline u64 capacity() const {
		return m_capacity;
	}

	inline u64 size() const {
		return m_capacity;
	}

	inline void swap(DynamicArray&& other) {
		m_data = other.m_data;
		m_capacity = other.m_capacity;
		other.m_data = nullptr;
		other.m_capacity = 0;
	}

	inline T& last() const {
		return m_data[m_capacity - 1];
	}

	inline operator T*() const {
		return m_data;
	}

	inline operator const T*() const {
		return m_data;
	}

private:
	T* m_data{};
	u64 m_capacity{};
};

}  // namespace toki
