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

	template <typename Callable, typename... Args>
	struct StateImpl : public State {
		using TupleType = Tuple<Callable, Args...>;

		StateImpl(Callable callable, Args... args):
			tuple(make_tuple<Callable, Args...>(toki::forward<Callable>(callable), toki::forward<Args>(args)...)) {}

		virtual void invoke() override {
			_invoke(MakeIndexSequence<TupleType::size>{});
		}

		template <u64... Is>
		void _invoke(IndexSequence<Is...>) {
			toki::invoke(get<Is>(toki::move(tuple))...);
		}

		TupleType tuple;
	};

public:
	Thread() = default;

	template <typename Callable, typename... Args>
		requires CIsCorrectCallable<Callable, void, Args...>
	explicit Thread(Callable&& callable, Args&&... args) {
		_start_thread(toki::forward<Callable>(callable), toki::forward<Args>(args)...);
	}

	DELETE_COPY(Thread)

	Thread(Thread&& other) {
		toki::swap(m_data, other.m_data);
	}
	Thread& operator=(Thread&& other) {
		toki::swap(m_data, other.m_data);
		return *this;
	}

	~Thread() {
		join();
	}

	template <typename Callable, typename... Args>
		requires CIsCorrectCallable<Callable, void, Args...>
	void start(Callable&& callable, Args&&... args) {
		_start_thread(toki::forward<Callable>(callable), toki::forward<Args>(args)...);
	}

	void join() {
		if (m_data.stack == nullptr) {
			return;
		}

		atomic_wait(&m_data.state->joinable, 0);
		_cleanup();
	}

private:
	template <typename Callable, typename... Args>
	void _start_thread(Callable&& callable, Args&&... args) {
		m_data.stack = DefaultAllocator::allocate_aligned(STACK_SIZE, 16);

		Tuple<Callable, Args...> tuple{ toki::forward<Callable>(callable), toki::forward<Args>(args)... };

		m_data.state = construct_at<StateImpl<Callable, Args...>>(
			m_data.stack, toki::forward<Callable>(callable), toki::forward<Args>(args)...);
		byte* stack_top = reinterpret_cast<byte*>(m_data.stack) + STACK_SIZE;

		_start_internal(stack_top, m_data.state);
	}

	void _start_internal(void* stack_top, void* ptr);
	void _wait_internal();

	void _cleanup() {
		_wait_internal();
		destroy_at(m_data.state);
		DefaultAllocator::free_aligned(m_data.stack);
	}

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

	struct {
		void* stack{};
		State* state{};
		i32 ctid{};
		i32 pid{};
	} m_data{};
};

}  // namespace toki
