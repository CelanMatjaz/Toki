#pragma once

#include "../string/string.h"
#include "types.h"

inline void* operator new(unsigned long, void* ptr) noexcept {
	return ptr;
}

namespace toki {

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

template <typename T>
inline T& remove_r_value_ref(T&& value) {
	return static_cast<T&>(value);
}

template <typename T>
inline constexpr T&& move(T& v) {
	return static_cast<T&&>(v);
}

template <typename T>
inline constexpr T&& move(const T& v) {
	return static_cast<T&&>(const_cast<T&>(v));
}

// Bootleg emplace
template <typename T, typename... Args>
inline T* emplace(void* dst, const Args&&... args) {
	return new (dst) T(move(args)...);
}

template <typename T>
inline constexpr void swap(T&& t1, T&& t2) {
	T temp = t1;
	t1 = t2;
	t2 = temp;
}

}  // namespace toki
