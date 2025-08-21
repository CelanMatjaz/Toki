#pragma once

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
			toki::memcpy(data, reinterpret_cast<byte*>(m_data) + sizeof(u64), element_count * sizeof(T));
		}
	}

	~BasicRef() {
		if (m_data == nullptr) {
			return;
		}

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

	inline void reset(u32 element_count = 1) {
		if (m_data != nullptr) {
			free();
		}

		alloc(element_count);
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
		// TK_ASSERT(index < m_capacity, "Index out of bounds");
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

	inline T& operator*() const {
		return *data();
	}

private:
	inline void swap(BasicRef<T>& other) {
		toki::swap(m_data, other.m_data);
		toki::swap(m_capacity, other.m_capacity);
	}

	void alloc(u32 element_count) {
		m_capacity = element_count;
		m_data = memory_allocate(element_count * sizeof(T) + sizeof(u64));
		reinterpret_cast<u64*>(m_data)[0] = 1;
	}

	void free() {
		--(*reinterpret_cast<u64*>(m_data));
		if (*reinterpret_cast<u64*>(m_data)) {
			memory_free(m_data);
		}
	}

	void* m_data{};
	u64 m_capacity{};
};

}  // namespace toki
