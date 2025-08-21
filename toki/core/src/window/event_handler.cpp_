#include "event_handler.h"

#include "memory/memory.h"

namespace toki {

EventQueue::EventQueue(): m_queue(memory_allocate_array<Event>(EVENT_QUEUE_SIZE), EVENT_QUEUE_SIZE) {}

b8 EventQueue::add_event(const Event& event) {
	TK_ASSERT_OR_RETURN(m_event_count < m_queue.size(), false);
	m_queue[m_event_count++] = event;
	return true;
}

WindowEventHandler::WindowEventHandler() {}

}  // namespace toki
