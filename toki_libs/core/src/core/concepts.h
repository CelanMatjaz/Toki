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
constexpr b8 IsIntegral = false;

#define IS_INTEGRAL(type) \
    template <>           \
    constexpr b8 IsIntegral<type> = true;

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
concept AllocatorConcept = requires(T a, u64 size, void* free_ptr) {
    { a.allocate(size) } -> SameAsConcept<void*>;
    { a.free(free_ptr) } -> SameAsConcept<void>;
};

}  // namespace toki
