#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
struct Formatter {};

}  // namespace toki
