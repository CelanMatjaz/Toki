#pragma once

#include "../core/assert.h"
#include "../memory/memory.h"

namespace toki {

template <typename T>
class DynamicArray {
public:
	DynamicArray() {}
	DynamicArray(u64 count): m_capacity(count), m_size(count), m_data(memory_allocate(sizeof(T) * count)) {}

	DynamicArray(u64 size, T&& default_value): DynamicArray(size) {
		for (u32 i = 0; i < size; i++) {
			m_data[i] = default_value;
		}
	}

	DynamicArray(DynamicArray&& other): m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
		other.m_data = nullptr;
		other.m_size = 0;
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
			move(toki::move(other));
		}
		return *this;
	}

	void resize(u32 new_size) {
		if (new_size <= m_capacity) {
			m_size = new_size;
			return;
		}

		m_data = memory_reallocate_array<T>(m_data, new_size);
		m_capacity = m_size = new_size;
	}

	void reserve(u32 new_capacity) {
		if (new_capacity <= m_capacity) {
			return;
		}

		m_data = memory_reallocate_array<T>(m_data, new_capacity);
		m_capacity = new_capacity;
	}

	inline void shrink_to_size(u32 new_size) {
		TK_ASSERT(new_size <= m_capacity, "New size cannot be larger than old size when shrinking");
		m_size = new_size;
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
		return m_size;
	}

	inline void move(DynamicArray&& other) {
		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	inline T& last() const {
		return m_data[m_size - 1];
	}

	inline operator T*() const {
		return m_data;
	}

	inline operator const T*() const {
		return m_data;
	}

private:
	T* m_data{};
	u64 m_size{};
	u64 m_capacity{};
};

}  // namespace toki
