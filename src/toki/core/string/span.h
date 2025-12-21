#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/types.h>

namespace toki {

template <typename T>
class Span {
public:
	Span(): m_data(nullptr), m_size(0) {}

	constexpr Span(const T* ptr, u64 size): m_data(ptr), m_size(size) {}

	template <typename ContainerType>
		requires CIsArrayContainer<const T, ContainerType>
	constexpr Span(const ContainerType& container): m_data(container.data()), m_size(container.size()) {}

	template <typename ContainerType>
		requires CIsArrayContainer<T, ContainerType>
	constexpr Span(ContainerType& container): m_data(container.data()), m_size(container.size()) {}

	template <typename ContainerType>
		requires(CIsBoundedArray<ContainerType>)
	constexpr Span(ContainerType container): m_data(container), m_size(IsBoundedArray<ContainerType>::COUNT) {}

	template <typename ContainerType>
		requires(CIsBoundedArray<ContainerType>)
	constexpr Span(ContainerType& container): m_data(container), m_size(IsBoundedArray<ContainerType>::COUNT) {}

	constexpr ~Span() = default;

	constexpr Span(const Span& other): m_data(other.m_data), m_size(other.m_size) {}

	constexpr Span& operator=(const Span& other) {
		if (&other == this) {
			return *this;
		}

		m_size = other.m_size;
		m_data = other.m_data;

		return *this;
	}

	constexpr Span(Span&& other): m_size(other.size), m_data(other.m_data) {
		other.m_size = 0;
		other.m_data = nullptr;
	}

	constexpr Span& operator=(Span&& other) {
		if (&other == this) {
			return *this;
		}

		m_size = other.m_size;
		m_data = other.m_data;

		return *this;
	}

	template <typename ContainerType>
		requires CIsArrayContainer<T, ContainerType>
	constexpr Span& operator=(ContainerType& container) {
		m_data = container.data();
		m_size = container.size();
		return *this;
	}

	constexpr u64 size() const {
		return m_size;
	}

	constexpr const T* data() const {
		return m_data;
	}

	const T& operator[](u64 index) const {
		return m_data[index];
	}

	const T& operator[](u64 index) {
		return const_cast<T*>(m_data)[index];
	}

private:
	const T* m_data{};
	u64 m_size{};
};

}  // namespace toki
