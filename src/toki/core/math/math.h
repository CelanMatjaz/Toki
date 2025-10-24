#pragma once

#include <toki/core/types.h>
#include <toki/core/common/assert.h>

namespace toki {

constexpr f64 PI = 3.14159265358979;
constexpr f64 TWO_PI = PI * 2;
constexpr const int TERMS = 10;

template <typename T>
constexpr T min(T v1, T v2) {
	return v1 < v2 ? v1 : v2;
}

template <typename T>
constexpr T max(T v1, T v2) {
	return v1 > v2 ? v1 : v2;
}

template <typename T>
constexpr T clamp(T value, T min_value, T max_value) {
	return min(max(value, min_value), max_value);
}

template <typename T>
constexpr T pow(T value, u32 exp) {
	if (exp == 0) {
		return 1;
	}

	T result = 1;
	for (u32 i = 0; i < exp; ++i) {
		result *= value;
	}

	return result;
}

template <typename T>
constexpr T abs(T value) {
	return value < 0 ? -value : value;
}

constexpr f64 sqrt(f64 value) {
	if (value < 0) {
		return -1;
	}
	if (value == 0) {
		return 0;
	}

#if defined(__has_builtin) && __has_builtin(__builtin_sqrt)
	return __builtin_sqrt(value);
#else
	f64 guess = value;
	constexpr u32 ITERATIONS = 10;

	for (u32 i = 0; i < ITERATIONS; i++) {
		guess = 0.5f * (guess + value / guess);
	}

	return guess;
#endif
}

template <typename T>
constexpr T hypot(const T& value1, const T& value2) {
	return sqrt(value1 * value1 + value2 * value2);
}

enum struct AngleUnits {
	Degrees,
	Radians
};

template <AngleUnits = AngleUnits::Radians>
constexpr f64 normalize_angle(f64 angle);

template <>
constexpr f64 normalize_angle<AngleUnits::Radians>(f64 angle) {
	while (angle > PI) {
		angle -= TWO_PI;
	}

	while (angle < -PI) {
		angle += TWO_PI;
	}

	return angle;
}

template <>
constexpr f64 normalize_angle<AngleUnits::Degrees>(f64 angle) {
	while (angle < 360.0f) {
		angle += 360.0f;
	}

	while (angle > 360.0f) {
		angle -= 360.0f;
	}

	return angle;
}

template <AngleUnits = AngleUnits::Radians>
constexpr f64 convert_angle_to(f64 angle);

template <>
constexpr f64 convert_angle_to<AngleUnits::Radians>(f64 angle) {
	f64 normalized = normalize_angle<AngleUnits::Degrees>(angle);
	return normalized * (PI / 180.0f);
}

template <>
constexpr f64 convert_angle_to<AngleUnits::Degrees>(f64 angle) {
	f64 normalized = normalize_angle<AngleUnits::Radians>(angle);
	return normalized * (180.0f / PI);
}

constexpr f64 factorial(u64 n) {
	f64 f = 1.0f;
	for (u64 i = 2; i <= n; i++) {
		f *= i;
	}
	return f;
}

// x - Angle in radians
constexpr f64 sin(f64 x) {
	x = normalize_angle(x);
	f64 result = 0.0f;
	for (int n = 0; n < TERMS; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2 * n + 1) / factorial(2 * n + 1);
	}
	return result;
}

// x - Angle in radians
constexpr f64 cos(f64 x) {
	x = normalize_angle(x);
	f64 result = 0.0f;
	for (int n = 0; n < TERMS; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2 * n) / factorial(2 * n);
	}
	return result;
}

// x - Angle in radians
constexpr f64 tan(f64 x) {
	f64 cos_value = cos(x);
	TK_ASSERT(cos_value != 0.0f);
	return sin(x) / cos_value;
}

// x - Angle in radians
constexpr f64 atan(f64 x) {
	f64 sin_value = sin(x);
	TK_ASSERT(sin_value != 0.0f);
	return cos(x) / sin_value;
}

}  // namespace toki
