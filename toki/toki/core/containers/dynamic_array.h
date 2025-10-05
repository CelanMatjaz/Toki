#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/common/print.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
class DynamicArray {
public:
	constexpr DynamicArray() {}

	constexpr DynamicArray(u64 count): m_data(nullptr) {
		reallocate(count);
	}

	constexpr DynamicArray(u64 count, T&& default_value): m_data(nullptr) {
		reallocate(count);
		for (u32 i = 0; i < count; i++) {
			m_data[i] = default_value;
		}
	}

	template <typename... Args>
		requires(Conjunction<IsSame<T, Args>...>::value && Conjunction<IsConvertible<Args, T>...>::value)
	constexpr DynamicArray(Args&&... args): DynamicArray(sizeof...(Args)) {
		u64 i = 0;
		((m_data[i++] = toki::forward<Args>(args)), ...);
	}

	constexpr DynamicArray(DynamicArray&& other):
		m_data(other.m_data),
		m_size(other.m_size),
		m_capacity(other.m_capacity) {
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	constexpr ~DynamicArray() {
		if (m_data != nullptr) {
			AllocatorType::free(m_data);
		}
	}

	DELETE_COPY(DynamicArray);

	constexpr DynamicArray& operator=(DynamicArray&& other) {
		if (this != &other) {
			move(toki::move(other));
		}
		return *this;
	}

	constexpr void resize(u64 new_size) {
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

	constexpr void reserve(u64 new_capacity) {
		if (new_capacity <= m_capacity) {
			return;
		}

		m_data = reinterpret_cast<T*>(AllocatorType::reallocate(m_data, new_capacity * sizeof(T)));
		m_capacity = new_capacity;
	}

	constexpr void shrink_to_size(u64 new_size) {
		TK_ASSERT(new_size <= m_capacity, "New size cannot be larger than old size when shrinking");
		m_size = new_size;
	}

	constexpr T& operator[](u64 index) const {
		return m_data[index];
	}

	constexpr const T* data() const {
		return m_data;
	}

	constexpr T* data() {
		return m_data;
	}

	constexpr u64 capacity() const {
		return m_capacity;
	}

	constexpr u64 size() const {
		return m_size;
	}

	constexpr void move(DynamicArray&& other) {
		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	constexpr T& last() const {
		return m_data[m_size - 1];
	}

	constexpr operator T*() const {
		return m_data;
	}

	constexpr operator const T*() const {
		return m_data;
	}

	template <typename... Args>
	constexpr void emplace_back(Args&&... args) {
		if (m_size >= m_capacity) {
			reallocate(toki::max<u64>(growth_factor * m_capacity, 1));
		}

		toki::construct_at<T>(&m_data[m_size], toki::forward<Args>(args)...);
	}

	constexpr void emplace_back(const T& value) {
		emplace_back(toki::forward(value));
	}

	constexpr void emplace_back(T&& value) {
		if (m_size >= m_capacity) {
			reallocate(toki::max<u64>(growth_factor * m_capacity, 1));
		}

		m_data[m_size++] = toki::move(value);
		return;
	}

	constexpr void clear() {
		if constexpr (CHasDestructor<T>) {
			for (u32 i = 0; i < m_size; i++) {
				toki::destroy_at<T>(&m_data[i]);
			}
		}

		m_size = 0;
	}

private:
	constexpr void reallocate(u64 new_capacity) {
		m_data = reinterpret_cast<T*>(AllocatorType::reallocate(m_data, new_capacity * sizeof(T)));
		m_capacity = new_capacity;
		m_size = new_capacity;
	}

	static constexpr const f32 growth_factor = 2.0f;

	T* m_data{};
	u64 m_size{};
	u64 m_capacity{};
};

}  // namespace toki
