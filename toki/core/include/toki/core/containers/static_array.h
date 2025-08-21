#pragma once

#include "../core/assert.h"
#include "../memory/memory.h"

namespace toki {

template <typename T, u64 SIZE>
class StaticArray {
public:
	StaticArray(): m_data(memory_allocate_array<T>(SIZE)) {}

	~StaticArray() {
		if (m_data != nullptr) {
			memory_free(m_data);
		}
	}

	StaticArray& operator=(StaticArray&& other) {
		if (this != &other) {
			m_data = other.m_data;
			other.m_data = nullptr;
		}

		return *this;
	}

	constexpr u64 size() const {
		return SIZE;
	}

	inline T* data() const {
		return m_data;
	}

	inline T& operator[](u64 index) const {
		return at(index);
	}

	inline T& at(u64 index) const {
		TK_ASSERT(index < SIZE, "Provided index is out of array bounds");
		return m_data[index];
	}

private:
	T* m_data = nullptr;
};

}  // namespace toki
