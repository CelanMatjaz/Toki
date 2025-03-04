#pragma once

#include "types.h"

namespace toki {

template <typename T1, typename T2>
constexpr bool isSameValue = false;

template <typename T>
constexpr bool isSameValue<T, T> = true;

template <typename T1, typename T2>
concept SameAsConcept = isSameValue<T1, T2>;

template <typename T>
concept AllocatorConcept = requires(T a, u64 size, void* free_ptr) {
    { a.allocate(size) } -> SameAsConcept<void*>;
    { a.free(free_ptr) } -> SameAsConcept<void>;
};

}  // namespace toki
