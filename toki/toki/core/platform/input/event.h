#pragma once

#include <toki/core/platform/input/event_defines.h>

namespace toki {

class Event {
public:
	Event() = default;
	Event(EventType type, EventData data);

	EventType type() const;
	EventData data() const;

	b8 handled() const;

	void set_handled();

private:
	EventType m_type : 7;
	b8 m_handled{};
	EventData m_data;
};

}  // namespace toki
