#pragma once

#include "../core/assert.h"
#include "../core/common.h"
#include "../core/types.h"
#include "../memory/memory.h"
#include "../string/string.h"

namespace toki {

template <typename T>
class BasicRef {
public:
	BasicRef() {};
	BasicRef(u32 element_count, const T* data = nullptr):
		m_data(memory_allocate(element_count * sizeof(T) + sizeof(u64))),
		m_capacity(element_count) {
		if (data != nullptr) {
			toki::memcpy(data, m_data, element_count * sizeof(T));
		}
	}

	~BasicRef() {
		--(*reinterpret_cast<u64*>(m_data));
		if (*reinterpret_cast<u64*>(m_data)) {
			memory_free(m_data);
		}
	}

	BasicRef(const BasicRef& other): m_data(other.m_data), m_capacity(other.m_capacity) {
		++(*reinterpret_cast<u64*>(m_data));
	}

	BasicRef& operator=(const BasicRef& other) {
		m_data = other.m_data;
		m_capacity = other.m_capacity;
		++(*reinterpret_cast<u64*>(m_data));
	}

	BasicRef(BasicRef<T>&& other) {
		if (this != &other) {
			this->swap(other);
		}
	}

	BasicRef& operator=(BasicRef&& other) {
		if (this != &other) {
			this->swap(remove_r_value_ref(other));
		}

		return *this;
	}

	// Count of items of type T that this ref can store
	inline u64 element_capacity() const {
		return m_capacity;
	}

	inline T* data() const {
		return reinterpret_cast<T*>(reinterpret_cast<u64*>(m_data) + 1);
	}

	inline operator bool() const {
		return m_data != nullptr;
	}

	inline bool valid() const {
		return m_data != nullptr;
	}

	T& operator[](u32 index) const {
		TK_ASSERT(index < m_capacity, "Index out of bounds");
		return data()[index];
	}

	inline T* operator->() const {
		return data();
	}

	inline operator T*() const {
		return data();
	}

	inline operator T&() const {
		return *data();
	}

private:
	inline void swap(BasicRef<T>& other) {
		toki::swap(m_data, other.m_data);
		toki::swap(m_capacity, other.m_capacity);
	}

	void* m_data{};
	u64 m_capacity{};
};

}  // namespace toki
