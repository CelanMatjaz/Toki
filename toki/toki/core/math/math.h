#pragma once

#include <toki/core/types.h>

namespace toki {

constexpr f64 PI = 3.14159265358979;
constexpr f64 TWO_PI = PI * 2;
constexpr const int TERMS = 10;

template <typename T>
inline constexpr T min(T v1, T v2) {
	return v1 < v2 ? v1 : v2;
}

template <typename T>
inline constexpr T max(T v1, T v2) {
	return v1 > v2 ? v1 : v2;
}

template <typename T>
inline constexpr T clamp(T value, T min_value, T max_value) {
	return min(max(value, min_value), max_value);
}

template <typename T>
inline constexpr T pow(T value, u32 exp) {
	if (exp == 0) {
		return 1;
	}

	while (--exp > 0) {
		value *= value;
	}

	return value;
}

template <typename T>
inline constexpr T abs(T value) {
	return value < 0 ? -value : value;
}

constexpr f32 sqrt(f32 value) {
	if (value < 0) {
		return -1;
	}
	if (value == 0) {
		return 0;
	}

#if 0 && defined(__has_builtin) && __has_builtin(__builtin_sqrt) 
	return __builtin_sqrt(value);
#else
	f32 guess = value;
	constexpr u32 ITERATIONS = 10;

	for (u32 i = 0; i < ITERATIONS; i++) {
		guess = 0.5f * (guess + value / guess);
	}

	return guess;
#endif
}

template <typename T>
inline constexpr T hypot(const T& value1, const T& value2) {
	return sqrt(value1 * value1 + value2 * value2);
}

enum class AngleUnits {
	Degrees,
	Radians
};

template <AngleUnits = AngleUnits::Radians>
constexpr f32 normalize_angle(f32 angle);

template <>
constexpr f32 normalize_angle<AngleUnits::Radians>(f32 angle) {
	while (angle < PI) {
		angle += TWO_PI;
	}

	while (angle > PI) {
		angle -= TWO_PI;
	}

	return angle;
}

template <>
constexpr f32 normalize_angle<AngleUnits::Degrees>(f32 angle) {
	while (angle < 360.0f) {
		angle += 360.0f;
	}

	while (angle > 360.0f) {
		angle -= 360.0f;
	}

	return angle;
}

template <AngleUnits = AngleUnits::Radians>
constexpr f32 convert_angle_to(f32 angle);

template <>
constexpr f32 convert_angle_to<AngleUnits::Radians>(f32 angle) {
	f32 normalized = normalize_angle<AngleUnits::Degrees>(angle);
	return normalized * (PI / 180.0f);
}

template <>
constexpr f32 convert_angle_to<AngleUnits::Degrees>(f32 angle) {
	f32 normalized = normalize_angle<AngleUnits::Radians>(angle);
	return normalized * (180.0f / PI);
}

constexpr f32 factorial(u32 n) {
	f32 f = 1.0f;
	for (u32 i = 2; i <= n; i++)
		f *= i;
	return f;
}

// x - Angle in radians
constexpr f32 sin(f32 x) {
	x = normalize_angle(x);
	f32 result = 0.0f;
	for (int n = 0; n < TERMS; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2 * n + 1) / factorial(2 * n + 1);
	}
	return result;
}

// x - Angle in radians
constexpr f32 cos(f32 x) {
	x = normalize_angle(x);
	f32 result = 0.0f;
	for (int n = 0; n < TERMS; n++) {
		int sign = (n % 2 == 0) ? 1 : -1;
		result += sign * pow(x, 2 * n) / factorial(2 * n);
	}
	return result;
}

}  // namespace toki
