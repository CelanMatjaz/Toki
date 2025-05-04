
#pragma once

#include "../string/span.h"
#include "event.h"

namespace toki {

constexpr u32 EVENT_QUEUE_SIZE = 1024;

class EventQueue {
public:
	EventQueue();

	inline b8 is_full() const {
		return m_event_count >= m_queue.size();
	}

	inline u64 count() const {
		return m_event_count;
	}

	const Event* events() const {
		return m_queue.data();
	}

	b8 add_event(const Event& event);

private:
	Span<Event> m_queue;
	u64 m_event_count{};
};

class WindowEventHandler {
public:
	WindowEventHandler();

	EventQueue& queue() {
		return m_queues[m_current_queue];
	}

	void swap() {
		m_current_queue = (m_current_queue + 1) & 2;
	}

private:
	EventQueue m_queues[2];
	u8 m_current_queue{};
};

}  // namespace toki
