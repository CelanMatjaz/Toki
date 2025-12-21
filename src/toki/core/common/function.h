#pragma once

#include <toki/core/common/common.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, typename... Args>
struct Function {};

template <typename T, typename... Args>
class Function<T(Args...)> {
public:
	using ReturnType	  = T;
	using FunctionPtrType = ReturnType (*)(Args...);

	~Function() {
		DefaultAllocator::free_aligned(m_callablePtr);
	}

	Function() = default;

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	Function(Callable callable) {
		init(callable);
	}

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	Function(FunctionPtrType func) {
		init(func);
	}

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	Function& operator=(Callable callable) {
		init(callable);
		return *this;
	}

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	Function& operator=(FunctionPtrType func) {
		init(func);
		return *this;
	}

	ReturnType operator()(Args&&... args) {
		return m_invoke(m_callablePtr, toki::forward<Args>(args)...);
	}

private:
	void* m_callablePtr{};
	ReturnType (*m_invoke)(const void*, Args...);

	template <FunctionPtrType Fn>
		requires CIsCorrectFn<ReturnType, Args...>
	void init(FunctionPtrType f) {
		m_callablePtr = f;
	}

	template <typename Callable>
		requires CIsCorrectCallable<Callable, ReturnType, Args...>
	void init(Callable c) {
		m_callablePtr = DefaultAllocator::allocate_aligned(sizeof(Callable), alignof(Callable));
		construct_at<Callable>(reinterpret_cast<decltype(c)*>(m_callablePtr), c);

		m_invoke = [](const void* self, Args... args) -> ReturnType {
			const Callable* callable = static_cast<const Callable*>(self);
			return (*callable)(args...);
		};
	}
};

}  // namespace toki
