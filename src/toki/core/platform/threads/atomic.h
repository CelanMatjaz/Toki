#pragma once

#include <toki/core/common/type_traits.h>

#include "toki/core/common/macros.h"

namespace toki {

#if defined(TOKI_CLANG) || defined(TOKI_GCC)

enum MemoryOrder {
	ATOMIC_RELAXED = __ATOMIC_RELAXED,
	// ATOMIC_CONSUME = __ATOMIC_CONSUME, // Should not be used
	ATOMIC_ACQUIRE = __ATOMIC_ACQUIRE,
	ATOMIC_RELEASE = __ATOMIC_RELEASE,
	ATOMIC_ACQ_REL = __ATOMIC_ACQ_REL,
	ATOMIC_SEQ_CST = __ATOMIC_SEQ_CST
};

inline i32 atomic_load(i32* t, MemoryOrder order = ATOMIC_SEQ_CST) {
	return __atomic_load_n(t, order);
}

inline void atomic_store(i32* t, const i32 value, MemoryOrder order = ATOMIC_SEQ_CST) {
	__atomic_store_n(t, value, order);
}

inline i32 atomic_exchange(i32* t, const i32 desired, MemoryOrder order = ATOMIC_SEQ_CST) {
	return __atomic_exchange_n(t, desired, order);
}

inline b8 atomic_compare_exchange_strong(i32* ptr,
										 i32* expected,
										 const i32 desired,
										 MemoryOrder success_order = ATOMIC_SEQ_CST,
										 MemoryOrder fail_order	   = ATOMIC_SEQ_CST) {
	return __atomic_compare_exchange_n(ptr, expected, desired, false, success_order, fail_order);
}

inline b8 atomic_compare_exchange_weak(i32* ptr,
									   i32* expected,
									   const i32 desired,
									   MemoryOrder success_order = ATOMIC_SEQ_CST,
									   MemoryOrder fail_order	 = ATOMIC_SEQ_CST) {
	return __atomic_compare_exchange_n(ptr, expected, desired, true, success_order, fail_order);
}

inline i32 atomic_fetch_add(i32* ptr, i32 add_value, MemoryOrder order = ATOMIC_SEQ_CST) {
	return __atomic_fetch_add(ptr, add_value, order);
}

inline i32 atomic_fetch_sub(i32* ptr, i32 subtract_value, MemoryOrder order = ATOMIC_SEQ_CST) {
	return __atomic_fetch_sub(ptr, subtract_value, order);
}

void atomic_wait(i32* addr, i32 old);

void atomic_notify_one(i32* addr);

void atomic_notify_all(i32* addr);

#endif

}  // namespace toki
