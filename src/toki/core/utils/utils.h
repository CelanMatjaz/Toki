#pragma once

#include <toki/core/common/function.h>
#include <toki/core/common/log.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/platform/defines.h>
#include <toki/core/platform/syscalls.h>
#include <toki/core/utils/path.h>

namespace toki {

template <auto... Values>
constexpr b8 any_of(auto value) {
	return ((value == Values) || ...);
}

template <typename T, typename DestroyFn>
	requires CIsCorrectCallable<DestroyFn, void, T&>
class LifetimeWrapper {
public:
	LifetimeWrapper(T&& obj, DestroyFn&& fn): m_obj(obj), m_fn(fn) {}
	~LifetimeWrapper() {
		m_fn(m_obj);
	}

	T& get() {
		return m_obj;
	}

	operator T&() {
		return m_obj;
	}

private:
	void _free() {
		m_obj = {};
		m_fn.cleanup();
	}

	T m_obj{};
	toki::Function<void(T&)> m_fn{};
};

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
	requires CIsArrayContainer<char, T>
TokiError dump_to_file(Path path, const T& container) {
	auto open_result = toki::open(path.c_str(), FileMode::WRITE, FileFlags::FILE_FLAG_CREATE);
	if (open_result.is_error()) {
		return open_result.error();
	}

	auto write_result = toki::write(open_result.value(), container.data(), container.size());
	if (write_result.is_error()) {
		return write_result.error();
	}

	return toki::close(open_result.value());
}

inline toki::Expected<u64, TokiError> read_line(NativeHandle handle, char* data, u64 count, byte delim) {
	u64 read_count{}, n{};
	char b{};
	for (n = 0; n < count; n++) {
		auto result = toki::read(handle, &b, 1);
		if (result.is_error()) {
			return toki::Unexpected(result.error());
		}

		if (read_count == 0) {
			return 0;
		} else if (read_count == 1) {
			*data++ = b;
			if (b == delim) {
				break;
			}
		}
	}

	return n;
}

}  // namespace toki
