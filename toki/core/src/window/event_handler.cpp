#include "event_handler.h"

namespace toki {

b8 EventQueue::add_event(const Event& event) {
	TK_ASSERT_OR_RETURN(m_event_count < m_queue.element_capacity(), false);
	m_queue[m_event_count++] = event;
	return true;
}

}  // namespace toki
