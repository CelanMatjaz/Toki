#include <toki/core/platform/input/event_handler.h>

namespace toki {

void EventHandler::dispatch_events(EventQueue events, void* sender) {
	for (Event& event : events) {
		for (u32 i = 0; i < m_receivers.size(); i++) {
			if (event.handled()) {
				break;
			}

			m_receivers[i].fn(sender, m_receivers[i].receiver, event);
		}
	}
}

}  // namespace toki
