#pragma once

#include <toki/core/common/type_traits.h>

namespace toki {

#if defined(TOKI_CLANG) || defined(TOKI_GCC)

inline i32 atomic_load(i32* t) {
	return __atomic_load_n(t, __ATOMIC_ACQUIRE);
}

inline void atomic_store(i32* t, const i32 value) {
	__atomic_store_n(t, value, __ATOMIC_RELEASE);
}

inline i32 atomic_exchange(i32* t, const i32 desired) {
	return __atomic_exchange_n(t, desired, __ATOMIC_ACQUIRE);
}

inline b8 atomic_compare_exchange_strong(i32* ptr, i32* expected, const i32 desired) {
	return __atomic_compare_exchange_n(ptr, expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
}

inline b8 atomic_compare_exchange_weak(i32* ptr, i32* expected, const i32 desired) {
	return __atomic_compare_exchange_n(ptr, expected, desired, true, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}

void atomic_wait(i32* addr, i32 old);

void atomic_notify_one(i32* addr);

void atomic_notify_all(i32* addr);

#endif

}  // namespace toki
