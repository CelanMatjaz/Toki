#pragma once

#include "types.h"

namespace toki {

template <typename T1, typename T2>
constexpr b8 IsSameValue = false;

template <typename T>
constexpr b8 IsSameValue<T, T> = true;

template <typename T1, typename T2>
concept SameAsConcept = IsSameValue<T1, T2>;

template <typename>
constexpr b8 IsIntegralValue = false;

#define IS_INTEGRAL(type) \
	template <>           \
	inline constexpr b8 IsIntegralValue<type> = true;

IS_INTEGRAL(b8)
IS_INTEGRAL(i8)
IS_INTEGRAL(u8)
IS_INTEGRAL(i16)
IS_INTEGRAL(u16)
IS_INTEGRAL(i32)
IS_INTEGRAL(u32)
IS_INTEGRAL(i64)
IS_INTEGRAL(u64)

#undef IS_INTEGRAL

template <typename T>
	requires IsIntegralValue<T>
constexpr b8 IsSignedValue = false;

#define IS_SIGNED(type) \
	template <>         \
	inline constexpr b8 IsSignedValue<type> = true;

IS_SIGNED(i8)
IS_SIGNED(i16)
IS_SIGNED(i32)
IS_SIGNED(i64)

#undef IS_SIGNED

template <typename T>
constexpr b8 IsCArray = false;

template <typename T, u64 N>
constexpr b8 IsCArray<T[N]> = true;

template <typename T, u64 N>
constexpr b8 IsCArray<const T (&)[N]> = true;

template <typename T>
constexpr b8 IsFloatingPoint = false;
template <>
inline constexpr b8 IsFloatingPoint<f32> = true;
template <>
inline constexpr b8 IsFloatingPoint<f64> = true;

template <typename T>
constexpr b8 IsPointer = false;
template <typename T>
constexpr b8 IsPointer<T*> = true;
template <typename T>
constexpr b8 IsPointer<const T*> = true;
template <typename T>
constexpr b8 IsPointer<T* volatile> = true;
template <typename T>
constexpr b8 IsPointer<const T* volatile> = true;

template <typename T>
concept AllocatorConcept = requires(T a, u64 size, void* free_ptr) {
	{ a.allocate(size) } -> SameAsConcept<void*>;
	{ a.free(free_ptr) } -> SameAsConcept<void>;
};

template <typename From, typename To>
concept IsConvertibleConcept = requires(From from, To to) {
	{ (To)(from) } -> SameAsConcept<To>;
};

template <typename T>
concept ToStringFunctionExistsConcept = requires(T t, char* buf) {
	{ to_string(buf, t) } -> SameAsConcept<u32>;
};

}  // namespace toki
