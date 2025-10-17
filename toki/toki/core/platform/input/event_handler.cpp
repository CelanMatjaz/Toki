#include <toki/core/platform/input/event_handler.h>

namespace toki {

void EventHandler::register_listener(void* listener, EventFunction fn) {
	m_listeners.emplace_back(listener, fn);
}

void EventHandler::unregister_listener(void* receiver) {
	for (u32 i = 0; i < m_listeners.size(); i++) {
		if (m_listeners[i].listener == receiver) {
			m_listeners.remove_at(i);
		}
	}

	TK_ASSERT(!"Receiver not registered");
}

void EventHandler::dispatch_event(const Event& event, void* sender) {
	for (u32 i = 0; i < m_listeners.size(); i++) {
		if (event.handled()) {
			break;
		}

		m_listeners[i].fn(sender, m_listeners[i].listener, event);
	}
}

}  // namespace toki
