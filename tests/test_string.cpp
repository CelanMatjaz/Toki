#include "testing.h"
//

#include <toki/core/core.h>

using namespace toki;

struct TempAllocator : DefaultAllocator {
	inline static Allocator* allocator = nullptr;
};

TK_TEST(BasicString, should_allocate_correctly) {
	Allocator allocator(GB(1));

	TempAllocator::allocator = &allocator;

	using TempString = toki::String<TempAllocator>;
	TempString string = "abcdefghijklmnopqrstuvwxyz";

	TempAllocator::allocator = nullptr;
	return true;
}

TK_TEST(BasicString, multiple_sized_strings) {
	Allocator allocator(GB(1));

	TempAllocator::allocator = &allocator;

	using TempString = toki::String<TempAllocator>;

	u32 string_size = 22;
	for (u32 i = 0; i < 100; i++) {
		if (i % 7 == 1) {
			auto b = TempString(string_size * i, i % 10 + '0');
			int a = 0;
		}
		auto b = TempString(string_size + i, i % 10 + '0');
		int a = 0;
	}

	TempAllocator::allocator = nullptr;
	return true;
}
