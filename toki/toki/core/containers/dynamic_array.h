#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/common/print.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/math/math.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
class DynamicArray {
public:
	DynamicArray() {}

	DynamicArray(u64 count): m_data(nullptr), m_size(count) {
		reallocate(count);
	}

	DynamicArray(u64 count, const T& default_value): m_data(nullptr), m_size(count) {
		reallocate(count);
		for (u64 i = 0; i < count; i++) {
			m_data[i] = default_value;
		}
	}

	template <typename... Args>
		requires(Conjunction<IsSame<T, Args>...>::value && Conjunction<IsConvertible<Args, T>...>::value)
	DynamicArray(Args&&... args): DynamicArray(sizeof...(Args)) {
		u64 i = 0;
		((m_data[i++] = toki::forward<Args>(args)), ...);
	}

	DynamicArray(DynamicArray&& other): m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	~DynamicArray() {
		if (m_data != nullptr) {
			AllocatorType::free(m_data);
		}
	}

	DELETE_COPY(DynamicArray);

	DynamicArray& operator=(DynamicArray&& other) {
		if (this != &other) {
			move(toki::move(other));
		}
		return *this;
	}

	void resize(u64 new_size) {
		if (new_size <= m_capacity) {
			m_size = new_size;
			return;
		}

		reallocate(new_size);

		m_size = new_size;

		if (m_size > m_capacity) {
			m_capacity = m_size;
		}
	}

	void reserve(u64 new_capacity) {
		if (new_capacity <= m_capacity) {
			return;
		}

		reallocate(new_capacity);
	}

	void shrink_to_size(u64 new_size) {
		TK_ASSERT(new_size <= m_capacity, "New size cannot be larger than old size when shrinking");
		m_size = new_size;
	}

	T& operator[](u64 index) const {
		return m_data[index];
	}

	const T* data() const {
		return m_data;
	}

	T* data() {
		return m_data;
	}

	u64 capacity() const {
		return m_capacity;
	}

	u64 size() const {
		return m_size;
	}

	u32 size_u32() const {
		return static_cast<u32>(m_size);
	}

	void move(DynamicArray&& other) {
		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	T& last() const {
		return m_data[m_size - 1];
	}

	operator T*() const {
		return m_data;
	}

	operator const T*() const {
		return m_data;
	}

	void push_back(const T& value) {
		maybe_allocate_for_new_element();
		toki::memcpy(&m_data[m_size++], &value, sizeof(T));
	}

	void push_back(T&& value) {
		maybe_allocate_for_new_element();
		m_data[m_size++] = toki::move(value);
	}

	template <typename... Args>
	void emplace_back(Args&&... args) {
		maybe_allocate_for_new_element();
		toki::construct_at<T>(&m_data[m_size++], toki::forward<Args>(args)...);
	}

	void clear() {
		if constexpr (CHasDestructor<T>) {
			for (u64 i = 0; i < m_size; i++) {
				toki::destroy_at<T>(&m_data[i]);
			}
		}

		m_size = 0;
	}

private:
	void maybe_allocate_for_new_element() {
		if (m_size >= m_capacity) {
			reallocate(toki::max<u64>(static_cast<u64>(growth_factor * static_cast<f32>(m_capacity)), 1));
		}
	}

	void reallocate(u64 new_capacity) {
		m_data = reinterpret_cast<T*>(AllocatorType::reallocate(m_data, static_cast<u64>(new_capacity * sizeof(T))));
		m_capacity = new_capacity;
	}

	static constexpr const f32 growth_factor = 2.0f;

	T* m_data{};
	u64 m_size{};
	u64 m_capacity{};
};

}  // namespace toki
