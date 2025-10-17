#include <toki/core/platform/input/event.h>

namespace toki {

Event::Event(EventType type, EventData data): m_type(type), m_data(data) {}

EventType Event::type() const {
	return m_type;
}

EventData Event::data() const {
	return m_data;
}

b8 Event::handled() const {
	return m_handled;
}

void Event::set_handled() {
	m_handled = true;
}

}  // namespace toki
