#pragma once

#include <toki/core/memory/memory.h>

#include "toki/core/common/assert.h"
#include "toki/core/common/common.h"
#include "toki/core/common/type_traits.h"

namespace toki {

template <typename T = void, CIsAllocator AllocatorType = DefaultAllocator>
class UniquePtr {
public:
	// template <typename... Args>
	// UniquePtr(Args&&... args): m_ptr(reinterpret_cast<T*>(AllocatorType::allocate(sizeof(T)))) {
	// 	TK_ASSERT(m_ptr != nullptr, "Internal pointer cannot be nullptr");
	// 	new (m_ptr) T(toki::forward<Args>(args)...);
	// }

	template <typename Derived>
	UniquePtr(UniquePtr<Derived>&& derived): m_ptr(derived.release()) {}

	constexpr UniquePtr(): m_ptr(nullptr) {}

	explicit constexpr UniquePtr(const T*& obj): m_ptr(obj) {}

	explicit constexpr UniquePtr(T*& obj): m_ptr(obj) {}

	explicit constexpr UniquePtr(T*&& obj): m_ptr(obj) {}

	explicit constexpr UniquePtr(UniquePtr&& other): m_ptr(other.m_ptr) {
		other.m_ptr = nullptr;
	}

	UniquePtr& operator=(UniquePtr&& other) {
		if (this == &other) {
			return *this;
		}

		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;

		return *this;
	}

	UniquePtr(const UniquePtr& other) = delete;

	UniquePtr& operator=(const UniquePtr& other) = delete;

	~UniquePtr() {
		reset();
	}

	void reset(T* ptr = nullptr) {
		if (m_ptr != nullptr) {
			static_cast<T*>(m_ptr)->T::~T();
			AllocatorType::free(m_ptr);
		}

		m_ptr = ptr;
	}

	[[nodiscard]] T* release() {
		T* ptr = m_ptr;
		m_ptr = nullptr;
		return ptr;
	}

	T* get() const {
		return m_ptr;
	}

	T* operator->() const {
		return m_ptr;
	}

private:
	T* m_ptr{};
};

template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args) {
	T* ptr = reinterpret_cast<T*>(DefaultAllocator::allocate_aligned(sizeof(T), alignof(T)));
	return UniquePtr<T>(construct_at<T>(ptr, toki::forward<Args>(args)...));
}

}  // namespace toki
