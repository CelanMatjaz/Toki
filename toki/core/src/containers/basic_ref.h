#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../memory/allocator.h"
#include "../string/string.h"
#include "../core/types.h"

namespace toki {

template <typename T, typename A = Allocator>
	requires AllocatorConcept<A>
class BasicRef {
public:
	BasicRef() {};
	BasicRef(A& allocator, u32 element_count = 1, const T* data = nullptr):
		m_allocator(&allocator),
		m_data(reinterpret_cast<T*>(allocator.allocate(element_count * sizeof(T) + sizeof(u64)))),
		m_capacity(element_count) {
		if (data != nullptr) {
			toki::memcpy(data, m_data, element_count * sizeof(T));
		}
	}

	~BasicRef() {
		--(*reinterpret_cast<u64*>(m_data));
		if (*reinterpret_cast<u64*>(m_data)) {
			m_allocator->free(m_data);
		}
	}

	BasicRef(const BasicRef& other): m_allocator(other.m_allocator), m_data(other.m_data), m_capacity(other.m_capacity) {
		++(*reinterpret_cast<u64*>(m_data));
	}

	BasicRef& operator=(const BasicRef& other) {
		m_allocator = other.m_allocator;
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
		return m_data;
	}

	inline operator T*() const {
		return m_data;
	}

	inline operator T&() const {
		return *m_data;
	}

private:
	inline void swap(BasicRef<T>& other) {
		toki::swap(m_allocator, other.m_allocator);
		toki::swap(m_data, other.m_data);
		toki::swap(m_capacity, other.m_capacity);
	}

	A* m_allocator{};
	T* m_data{};
	u64 m_capacity{};
};

class BumpAllocator;

template <typename T>
using BumpRef = BasicRef<T, BumpAllocator>;

}  // namespace toki
