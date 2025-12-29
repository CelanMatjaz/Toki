#pragma once

#include <toki/core/common/common.h>
#include <toki/core/memory/memory.h>

#include "toki/core/common/assert.h"

namespace toki {

template <typename T, typename... Args>
struct Function {};

template <typename T, typename... Args>
class Function<T(Args...)> {
public:
	using ReturnType	  = T;
	using FunctionPtrType = ReturnType (*)(Args...);

	~Function() {
		_cleanup();
	}

	Function() = default;

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	Function(Callable callable) {
		_init(callable);
	}

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	Function(FunctionPtrType func) {
		_init(func);
	}

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	Function& operator=(Callable callable) {
		_init(callable);
		return *this;
	}

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	Function& operator=(FunctionPtrType func) {
		_init(func);
		return *this;
	}

	void cleanup() {
		_cleanup();
	}

	ReturnType operator()(Args&&... args) {
		TK_ASSERT(m_callablePtr != nullptr);
		return m_invoke(m_callablePtr, toki::forward<Args>(args)...);
	}

private:
	void* m_callablePtr{};
	ReturnType (*m_invoke)(const void*, Args...);

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	void _init(FunctionPtrType f) {
		m_callablePtr = f;
	}

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	void _init(Callable c) {
		m_callablePtr = DefaultAllocator::allocate_aligned(sizeof(Callable), alignof(Callable));
		construct_at<Callable>(reinterpret_cast<decltype(c)*>(m_callablePtr), c);

		m_invoke = [](const void* self, Args... args) -> ReturnType {
			const Callable* callable = static_cast<const Callable*>(self);
			return (*callable)(args...);
		};
	}

	void _cleanup() {
		DefaultAllocator::free_aligned(m_callablePtr);
		m_callablePtr = nullptr;
	}
};

}  // namespace toki
