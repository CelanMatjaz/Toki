#pragma once

#include <toki/core/common/common.h>
#include <toki/core/common/macros.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, typename AllocatorType = DefaultAllocator>
class UniqueRef {
public:
	UniqueRef(): m_data(nullptr) {}

	UniqueRef(const T&& value) {
		init(toki::move(value));
	}

	template <typename... Args>
	UniqueRef(const Args&&... args) {
		init(toki::move(args)...);
	}

	DELETE_COPY(UniqueRef)

	~UniqueRef() {
		reset();
	}

	UniqueRef(UniqueRef<T>&& other) {
		if (this != &other) {
			this->swap(other);
		}
	}

	UniqueRef& operator=(UniqueRef&& other) {
		if (this != &other) {
			this->swap(remove_r_value_ref(other));
		}

		return *this;
	}

	void init(const T&& value) {
		m_data	= AllocatorType::allocate(sizeof(T));
		*m_data = toki::move(value);
	}

	template <typename... Args>
	void init(const Args&&... args) {
		m_data	= AllocatorType::allocate(sizeof(T));
		emplace<T>(m_data, toki::move(args)...);
	}

	void reset() {
		if (m_data != nullptr) {
			(*m_data).~T();
			AllocatorType::free(m_data);
			m_data = nullptr;
		}
	}

	inline T* data() const {
		return m_data;
	}

	inline operator bool() const {
		return m_data != nullptr;
	}

	inline bool valid() const {
		return m_data != nullptr;
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
	inline void swap(UniqueRef<T>& other) {
		this->m_data = other.m_data;
		other.m_data = nullptr;
	}

	T* m_data{};
};

}  // namespace toki
