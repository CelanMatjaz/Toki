#pragma once

#include <toki/core/containers/dynamic_array.h>
#include <toki/core/memory/memory.h>
#include <toki/core/string/span.h>

namespace toki {

inline u64 _get_pivot_index(u64 start, u64 end) {
	return ((end - start) >> 1) + start;
}

template <typename T>
inline void _qsort_subregion(T* array, u64 start, u64 end) {
	if (start >= end) {
		return;
	}

	u64 pivot_index = _get_pivot_index(start, end);
	swap(array[pivot_index], array[end]);
	T pivot = array[end];

	i64 left  = static_cast<i64>(start);
	i64 right = static_cast<i64>(end) - 1;

	while (left <= right) {
		while (array[left] < pivot) {
			++left;
		}
		while (array[right] > pivot) {
			--right;
		}

		if (left <= right) {
			swap(array[left], array[right]);
			++left;
			--right;
		}
	}

	swap(array[static_cast<u64>(left)], array[end]);

	if (left > static_cast<i64>(start)) {
		_qsort_subregion(array, start, static_cast<u64>(left) - 1);
	}
	_qsort_subregion(array, static_cast<u64>(left) + 1, end);
}

template <typename T>
inline void qsort(T* data, u64 size) {
	TK_ASSERT(size > 0);
	TK_ASSERT(reinterpret_cast<u64ptr>(data) % alignof(T) == 0);
	_qsort_subregion(data, 0, size - 1);
}

template <typename T, typename AllocatorType = DefaultAllocator>
DynamicArray<T, AllocatorType> to_qsorted(Span<T> items) {
	DynamicArray<T, AllocatorType> array(items.size());
	qsort(array.data(), array.size());
	toki::memcpy(array.data(), items.data(), items.size() * sizeof(T));
	return array;
}

}  // namespace toki
