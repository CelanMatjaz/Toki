#pragma once

#include <toki/core/common/type_traits.h>

namespace toki {

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

template <typename T>
inline T& remove_r_value_ref(T&& value) {
	return static_cast<T&>(value);
}

template <typename T>
inline RemoveRef<T>::type&& remove_ref(T&& value) {
	return static_cast<RemoveRef<T>::type&&>(value);
}

template <typename T>
inline constexpr RemoveRef<T>::type&& move(T&& v) {
	return static_cast<typename RemoveRef<T>::type&&>(v);
}

template <typename T>
inline constexpr T&& forward(typename RemoveRef<T>::type& t) {
	return static_cast<T&&>(t);
}

template <typename T>
	requires(!CIsLValueReference<T>)
inline constexpr T&& forward(typename RemoveRef<T>::type&& t) {
	return static_cast<T&&>(t);
}

template <typename T>
inline constexpr void swap(T&& t1, T&& t2) {
	T temp = t1;
	t1 = t2;
	t2 = temp;
}

template <typename T, typename... Args>
inline T* construct_at(T* dst, Args&&... args) {
	return ::new (static_cast<void*>(dst)) T(toki::forward<Args>(args)...);
}

template <typename T>
	requires(!CIsCArray<T> && CHasDestructor<T>)
inline constexpr void destroy_at(T* t) {
	t->T::~T();
}

template <typename To, typename From>
To convert_to(const From& from);

}  // namespace toki
