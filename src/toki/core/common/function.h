#pragma once

#include <toki/core/common/common.h>
#include <toki/core/memory/memory.h>

namespace toki {

template <typename T, typename... Args>
struct Function {};

template <typename T, typename... Args>
class Function<T(Args...)> {
public:
	using ResultType = T;

	~Function() {
		DefaultAllocator::free_aligned(m_callablePtr);
	}

	template <typename Callable>
	Function(Callable callable) {
		init(callable);
	}

	template <typename Callable>
	Function& operator=(Callable callable) {
		init(callable);
		return *this;
	}

	ResultType operator()(Args&&... args) {
		return m_invoke(m_callablePtr, toki::forward<Args>(args)...);
	}

private:
	void* m_callablePtr{};
	ResultType (*m_invoke)(const void*, Args...);

	template <typename Callable>
	void init(Callable c) {
		m_callablePtr = DefaultAllocator::allocate_aligned(sizeof(Callable), alignof(Callable));
		construct_at<Callable>(reinterpret_cast<decltype(c)*>(m_callablePtr), c);

		m_invoke = [](const void* self, Args... args) -> ResultType {
			const Callable* callable = static_cast<const Callable*>(self);
			return (*callable)(args...);
		};
	}

	ResultType (*m_functionPtr)(Args...);
};

}  // namespace toki
