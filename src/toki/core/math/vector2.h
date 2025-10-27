#pragma once

#include <toki/core/math/math.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/types.h>
#include <toki/core/utils/format.h>
#include <toki/core/utils/string_formatters.h>

namespace toki {

template <typename T>
class _vector2 {
public:
	constexpr _vector2() = default;
	constexpr _vector2(T value): x(value), y(value) {}
	constexpr _vector2(T x_value, T y_value): x(x_value), y(y_value) {}
	constexpr _vector2(const _vector2&) = default;
	constexpr _vector2(_vector2&&) = default;

	constexpr _vector2& operator=(const _vector2&) = default;
	constexpr _vector2& operator=(_vector2&&) = default;

	constexpr _vector2& operator+=(const _vector2& rhs);
	constexpr _vector2 operator+(const _vector2& rhs) const;

	constexpr _vector2& operator-=(const _vector2& rhs);
	constexpr _vector2 operator-(const _vector2& rhs) const;

	constexpr _vector2& operator*=(const _vector2& rhs);
	constexpr _vector2 operator*(const _vector2& rhs) const;

	b8 operator<=>(const _vector2&) const = default;

	constexpr f32 length_squared() const;
	constexpr f32 length() const;
	constexpr _vector2 normalize() const;

public:
	T x, y;
};

template <typename T>
constexpr _vector2<T>& _vector2<T>::operator+=(const _vector2<T>& rhs) {
	x += rhs.x;
	y += rhs.y;

	return *this;
}

template <typename T>
inline constexpr _vector2<T> _vector2<T>::operator+(const _vector2<T>& rhs) const {
	_vector2 temp(*this);
	return (temp += rhs);
}

template <typename T>
constexpr _vector2<T>& _vector2<T>::operator-=(const _vector2<T>& rhs) {
	x -= rhs.x;
	y -= rhs.y;

	return *this;
}

template <typename T>
inline constexpr _vector2<T> _vector2<T>::operator-(const _vector2& rhs) const {
	_vector2 temp(*this);
	return (temp -= rhs);
}

template <typename T>
constexpr _vector2<T>& _vector2<T>::operator*=(const _vector2<T>& rhs) {
	x *= rhs.x;
	y *= rhs.y;

	return *this;
}

template <typename T>
inline constexpr _vector2<T> _vector2<T>::operator*(const _vector2& rhs) const {
	_vector2 temp(*this);
	return (temp *= rhs);
}

template <typename T>
constexpr f32 _vector2<T>::length_squared() const {
	return x * x + y * y;
}

template <typename T>
constexpr f32 _vector2<T>::length() const {
	return toki::sqrt(length_squared());
}

template <typename T>
constexpr _vector2<T> _vector2<T>::normalize() const {
	f32 len = length();
	_vector2 vector = *this;
	vector.x /= len;
	vector.y /= len;

	return vector;
}

template <typename T, CIsAllocator AllocatorType>
struct Formatter<_vector2<T>, AllocatorType> {
	static constexpr toki::String<AllocatorType> format(const _vector2<T>& vector) {
		return toki::format("Vector2 [{} {} {}]", vector.x, vector.y, vector.z);
	}
};

using Vector2 = _vector2<f32>;
using Vector2u32 = _vector2<u32>;
using Vector2i32 = _vector2<i32>;

}  // namespace toki
