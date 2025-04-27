
#pragma once

#include "containers/basic_ref.h"
#include "core/concepts.h"
#include "event.h"

namespace toki {

class EventQueue {
public:
	EventQueue(BumpRef<Event>& queue);

	inline b8 is_full() const {
		return m_event_count >= m_queue.element_capacity();
	}

	inline u64 count() const {
		return m_event_count;
	}

	const Event* events() const {
		return m_queue.data();
	}

	b8 add_event(const Event& event);

private:
	BumpRef<Event>& m_queue;
	u64 m_event_count{};
};

template <typename A = Allocator>
	requires AllocatorConcept<A>
class WindowEventHandler {
public:
	WindowEventHandler() = delete;
	WindowEventHandler(A& allocator);

private:
	A& m_allocator;
};

}  // namespace toki
