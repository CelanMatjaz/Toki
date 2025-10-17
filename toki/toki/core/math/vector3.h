#pragma once

#include <toki/core/math/math.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/types.h>
#include <toki/core/utils/format.h>
#include <toki/core/utils/string_formatters.h>

namespace toki {

class Vector3 {
public:
	constexpr Vector3() = default;
	constexpr Vector3(f32 value): x(value), y(value), z(value) {}
	constexpr Vector3(f32 x_value, f32 y_value, f32 z_value): x(x_value), y(y_value), z(z_value) {}
	constexpr Vector3(const Vector3&) = default;
	constexpr Vector3(Vector3&&) = default;

	constexpr Vector3& operator=(const Vector3&) = default;
	constexpr Vector3& operator=(Vector3&&) = default;

	constexpr b8 operator==(const Vector3&) const = default;

	friend constexpr Vector3& operator+=(Vector3& lhs, const Vector3& rhs);
	constexpr Vector3 operator+(const Vector3& rhs) const;

	friend constexpr Vector3& operator-=(Vector3& lhs, const Vector3& rhs);
	constexpr Vector3 operator-(const Vector3& rhs) const;

	friend constexpr Vector3& operator*=(Vector3& lhs, const Vector3& rhs);
	constexpr Vector3 operator*(const Vector3& rhs) const;
	constexpr Vector3 operator*(f32 value) const;

	constexpr Vector3 operator-() const;

	constexpr f32 length_squared() const;
	constexpr f32 length() const;
	constexpr Vector3 normalize() const;
	constexpr Vector3 cross(const Vector3& v) const;
	constexpr f32 dot(const Vector3& v) const;

public:
	f32 x{}, y{}, z{};
};

constexpr Vector3& operator+=(Vector3& lhs, const Vector3& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;

	return lhs;
}

inline constexpr Vector3 Vector3::operator+(const Vector3& rhs) const {
	Vector3 temp(*this);
	return (temp += rhs);
}

constexpr Vector3& operator-=(Vector3& lhs, const Vector3& rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;

	return lhs;
}

inline constexpr Vector3 Vector3::operator-(const Vector3& rhs) const {
	Vector3 temp(*this);
	return (temp -= rhs);
}

constexpr Vector3& operator*=(Vector3& lhs, const Vector3& rhs) {
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;

	return lhs;
}

inline constexpr Vector3 Vector3::operator*(const Vector3& rhs) const {
	Vector3 temp(*this);
	return (temp *= rhs);
}

constexpr Vector3 Vector3::operator*(f32 value) const {
	Vector3 temp(*this);
	temp.x *= value;
	temp.y *= value;
	temp.z *= value;
	return temp;
}

constexpr Vector3 Vector3::operator-() const {
	return Vector3(-x, -y, -z);
}

constexpr f32 Vector3::length_squared() const {
	return x * x + y * y + z * z;
}

constexpr f32 Vector3::length() const {
	return toki::sqrt(length_squared());
}

constexpr Vector3 Vector3::normalize() const {
	f32 len = length();
	Vector3 vector = *this;
	vector.x /= len;
	vector.y /= len;
	vector.z /= len;

	return vector;
}

constexpr Vector3 Vector3::cross(const Vector3& other) const {
	return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}

constexpr f32 Vector3::dot(const Vector3& other) const {
	return (x * other.x + y * other.y + z * other.z);
}

template <>
struct Formatter<Vector3> {
	static constexpr toki::String format(const Vector3& vector) {
		return toki::format("Vector3 [{} {} {}]", vector.x, vector.y, vector.z);
	}
};

}  // namespace toki
