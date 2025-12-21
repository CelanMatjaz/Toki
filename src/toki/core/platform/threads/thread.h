#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/common/function.h>
#include <toki/core/common/utility.h>
#include <toki/core/containers/tuple.h>
#include <toki/core/memory/unique_ptr.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/platform/threads/atomic.h>
#include <toki/core/platform/threads/mutex.h>
#include <toki/core/utils/memory.h>

namespace toki {

using ThreadHandle = i32;

class Thread {
private:
	struct State {
		virtual ~State()	  = default;
		virtual void invoke() = 0;

		i32 joinable{};
	};

	template <typename Callable>
	struct StateImpl : public State {
		Callable func;

		template <typename... Args>
		StateImpl(Args&&... args): func(toki::forward<Args>(args)...) {}

		virtual void invoke() override {
			func();
		}
	};

	template <typename TupleType>
	struct Invoker {
	public:
		template <typename... Args>
		explicit Invoker(Args&&... args): tuple(toki::forward<Args>(args)...) {}

		template <u64... Is>
		auto invoke(IndexSequence<Is...>) {
			return toki::invoke(get<Is>(toki::move(tuple))...);
		}

		auto operator()() {
			return invoke(MakeIndexSequence<TupleType::size>{});
		}

	public:
		TupleType tuple;
	};

public:
	Thread() = default;

	template <typename Callable, typename... Args>
		requires CIsCorrectCallable<Callable, void, Args...>
	explicit Thread(Callable&& callable, Args&&... args) {
		start_thread(toki::forward<Callable>(callable), toki::forward<Args>(args)...);
	}

	~Thread() {
		join();
	}

	template <typename... Args>
	void start(Function<void(Args...)> fn, Args&&... args) {
		start_thread(fn, toki::forward<Args>(args)...);
	}

	template <typename Callable, typename... Args>
		requires CIsCorrectCallable<Callable, void, Args...>
	void start(Callable&& callable, Args&&... args) {
		start_thread(toki::forward<Callable>(callable), toki::forward<Args>(args)...);
	}

	void join() {
		atomic_wait(&m_state->joinable, 0);
	}

private:
	template <typename Callable, typename... Args>
	void start_thread(Callable&& callable, Args&&... args) {
		m_stack = DefaultAllocator::allocate_aligned(STACK_SIZE, 16);

		m_state = construct_at<StateImpl<Invoker<Tuple<Callable, Args...>>>>(
			m_stack, toki::forward<Callable>(callable), toki::forward<Args>(args)...);
		byte* stack_top = reinterpret_cast<byte*>(m_stack) + STACK_SIZE;

		_start_internal(stack_top, m_state);
	}

	void _start_internal(void* stack_top, void* ptr);

	static int _trampoline(void* ptr) {
		State* state = reinterpret_cast<State*>(ptr);

		atomic_store(&state->joinable, static_cast<i32>(0));

		state->invoke();

		atomic_store(&state->joinable, static_cast<i32>(1));
		atomic_notify_all(&state->joinable);

		return 0;
	}

private:
	static constexpr const u64 STACK_SIZE = 1024 * 1024;

	void* m_stack{};
	State* m_state{};
	i64 m_pid{};
};

}  // namespace toki
