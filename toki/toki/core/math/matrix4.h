#pragma once

#include <toki/core/containers/array.h>
#include <toki/core/math/vector3.h>

#include "toki/core/common/type_traits.h"

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

	constexpr Matrix4(const Vector3& vec);

	constexpr Matrix4(const Matrix4&);
	constexpr Matrix4& operator=(const Matrix4&);

	constexpr b8 operator==(const Matrix4&) const = default;

	constexpr Matrix4& operator*=(const Matrix4& other);
	constexpr Matrix4 operator*(const Matrix4& other) const;

	friend Formatter<Matrix4>;

private:
	Array<f32, 16> m{};
};

constexpr Matrix4::Matrix4(): m(0) {
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

constexpr Matrix4::Matrix4(const Vector3& vec): Matrix4() {
	m[12] = vec.x;
	m[13] = vec.y;
	m[14] = vec.z;
}

constexpr Matrix4::Matrix4(const Matrix4& other): m(other.m) {}

constexpr Matrix4& Matrix4::operator=(const Matrix4& other) {
	if (*this == other) {
		return *this;
	}

	m = other.m;
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
