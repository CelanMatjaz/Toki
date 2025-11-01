#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/memory/memory.h>
#include <toki/core/string/basic_string.h>

namespace toki {

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
struct Formatter {
	inline static constexpr const char* format_string{};
};

template <typename Type, typename AllocatorType = DefaultAllocator>
concept CHasStringFormatter = requires(Type t, char* buf_out, const Type& tref) {
	{ Formatter<Type, AllocatorType>::format(t) } -> CIsSame<toki::String<AllocatorType>>;
	{ Formatter<Type, AllocatorType>::format_to(buf_out, tref) } -> CIsSame<u64>;
};

}  // namespace toki
