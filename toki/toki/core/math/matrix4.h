#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/containers/array.h>
#include <toki/core/math/vector3.h>

namespace toki {

class Matrix4 {
public:
	constexpr Matrix4();
	constexpr Matrix4(const f32& value): m(value) {}

	template <typename... Args>
		requires(sizeof...(Args) == 16 && CConjunction<IsConvertible<Args, f32>...>)
	constexpr Matrix4(const Args&... args): m{ static_cast<f32>(toki::forward<Args>(args))... } {}

	template <typename... Args>
		requires(sizeof...(Args) == 16 && CConjunction<IsConvertible<Args, f32>...>)
	constexpr Matrix4(Args&&... args): m{ static_cast<f32>(toki::forward<Args>(args))... } {}

	constexpr Matrix4(const Vector3& position);

	constexpr Matrix4(const Matrix4&);
	constexpr Matrix4& operator=(const Matrix4&);

	constexpr b8 operator==(const Matrix4&) const = default;

	constexpr Matrix4& operator*=(const Matrix4& other);
	constexpr Matrix4 operator*(const Matrix4& other) const;

	constexpr Matrix4 translate(const Vector3& position) const;
	constexpr Matrix4 scale(f32 uniform_scale) const;
	constexpr Matrix4 scale(const Vector3& scale) const;
	constexpr Matrix4 rotate(const Vector3& axis, f32 angle) const;

	friend Formatter<Matrix4>;

private:
	Array<f32, 16> m{};
};

constexpr Matrix4::Matrix4() {
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

constexpr Matrix4::Matrix4(const Vector3& position): Matrix4() {
	m[12] = position.x;
	m[13] = position.y;
	m[14] = position.z;
}

constexpr Matrix4::Matrix4(const Matrix4& other) {
	for (u32 i = 0; i < 16; ++i) {
		m[i] = other.m[i];
	}
}

constexpr Matrix4& Matrix4::operator=(const Matrix4& other) {
	if (*this != other) {
		for (u32 i = 0; i < 16; ++i) {
			m[i] = other.m[i];
		}
	}

	return *this;
}

constexpr Matrix4& Matrix4::operator*=(const Matrix4& other) {
	*this = *this * other;
	return *this;
}

constexpr Matrix4 Matrix4::operator*(const Matrix4& other) const {
	Matrix4 result;

	for (u32 col = 0; col < 4; col++) {
		for (u32 row = 0; row < 4; row++) {
			f32 sum = 0.0f;
			for (u32 k = 0; k < 4; k++) {
				sum += m[k * 4 + row] * other.m[col * 4 + k];
			}
			result.m[col * 4 + row] = sum;
		}
	}

	return result;
}

constexpr Matrix4 Matrix4::translate(const Vector3& position) const {
	Matrix4 temp;
	temp.m[12] = position.x;
	temp.m[13] = position.y;
	temp.m[14] = position.z;
	return *this * temp;
}

constexpr Matrix4 Matrix4::scale(f32 uniform_scale) const {
	Matrix4 temp;
	temp.m[0] = uniform_scale;
	temp.m[5] = uniform_scale;
	temp.m[10] = uniform_scale;
	return *this * temp;
}

constexpr Matrix4 Matrix4::scale(const Vector3& scale) const {
	Matrix4 temp;
	temp.m[0] += scale.x;
	temp.m[5] += scale.y;
	temp.m[10] += scale.z;
	return *this * temp;
}

constexpr Matrix4 Matrix4::rotate(const Vector3& axis, f32 angle) const {
	Vector3 a = axis.normalize();
	f32 c = cos(angle);
	f32 s = sin(angle);
	f32 t = 1.0f - c;

	f32 x = a.x;
	f32 y = a.y;
	f32 z = a.z;

	return *this * Matrix4(
					   t * x * x + c,
					   t * x * y + s * z,
					   t * x * z - s * y,
					   0.0f,
					   t * x * y - s * z,
					   t * y * y + c,
					   t * y * z + s * x,
					   0.0f,
					   t * x * z + s * y,
					   t * y * z - s * x,
					   t * z * z + c,
					   0.0f,
					   0.0f,
					   0.0f,
					   0.0f,
					   1.0f);
}

template <>
struct Formatter<Matrix4> {
	static constexpr toki::String format(const Matrix4& matrix) {
		return toki::format(
			"Matrix4\n[{} {} {} {}]\n[{} {} {} {}]\n[{} {} {} {}]\n[{} {} {} {}]",
			matrix.m[0],
			matrix.m[4],
			matrix.m[8],
			matrix.m[12],
			matrix.m[1],
			matrix.m[5],
			matrix.m[9],
			matrix.m[13],
			matrix.m[2],
			matrix.m[6],
			matrix.m[10],
			matrix.m[14],
			matrix.m[3],
			matrix.m[7],
			matrix.m[11],
			matrix.m[15]);
	}
};

}  // namespace toki
