#include <toki/core/core.h>

#include "testing.h"

using namespace toki;

template <typename T>
b8 check_array(Span<T> arr) {
	for (u32 i = 0; i < arr.size() - 1; i++) {
		if (arr[i] > arr[i + 1]) {
			return false;
		}
	}

	return true;
}

TK_TEST(QSort, single_element) {
	u32 array[] = { 0 };
	qsort(array, ARRAY_SIZE(array));

	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}

TK_TEST(QSort, reverse_sorted) {
	u32 array[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}

TK_TEST(QSort, already_sorted) {
	u32 array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}

TK_TEST(QSort, many_duplicates) {
	u32 array[] = { 4, 1, 4, 2, 4, 3, 4, 4, 3, 1 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}

TK_TEST(QSort, signed_integers) {
	i32 array[] = { -3, 5, 0, -1, 7, -8, 2, 4, -5 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<i32>(array));

	return true;
}

TK_TEST(QSort, 2_elements) {
	u32 array[] = { 1, 0 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}

TK_TEST(QSort, many_elements_with_duplicates_and_negatives) {
	i32 array[] = { 10, -10, 10, -10, 0, 0, 5, -5, 5, -5 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<i32>(array));

	return true;
}

TK_TEST(QSort, identical_elements) {
	u32 array[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	qsort(array, ARRAY_SIZE(array));
	TK_TEST_ASSERT(check_array<u32>(array));

	return true;
}
