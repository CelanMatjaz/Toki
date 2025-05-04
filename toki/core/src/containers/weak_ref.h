#pragma once

#include "../core/common.h"
#include "../core/macros.h"
#include "../memory/memory.h"

namespace toki {

template <typename T>
class WeakRef {
public:
	WeakRef(): m_data(nullptr) {}
	WeakRef(T* ref): m_data(ref) {}

	template <typename... Args>
	WeakRef(const Args&&... args) {
		init(toki::move(args)...);
	}

	WeakRef(WeakRef<T>&& other) {
		if (this != &other) {
			this->swap(other);
		}
	}

	WeakRef& operator=(WeakRef&& other) {
		if (this != &other) {
			this->swap(remove_r_value_ref(other));
		}

		return *this;
	}

	WeakRef(const WeakRef& other): m_data(other.m_data) {}

	WeakRef& operator=(const WeakRef& other) {
		this->m_data = other.m_data;
		return *this;
	}

	void reset() {
		if (m_data != nullptr) {
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
	inline void swap(WeakRef<T>& other) {
		this->m_data = other.m_data;
		other.m_data = nullptr;
	}

	T* m_data{};
};

}  // namespace toki
