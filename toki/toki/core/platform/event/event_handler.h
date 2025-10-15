#pragma once

#include <toki/core/containers/arena.h>
#include <toki/core/containers/dynamic_array.h>
#include <toki/core/containers/ring_buffer.h>
#include <toki/core/platform/event/event.h>

namespace toki {

constexpr u32 EVENT_ARENA_QUEUE_ELEMENT_COUNT = 128;
using EventQueue = toki::RingBuffer<Event, EVENT_ARENA_QUEUE_ELEMENT_COUNT>;

struct EventReceiver {
	void* receiver{};
	void* (&fn)(void* sender, void* receiver, Event& event);
};

class EventHandler {
public:
	EventHandler() = default;

	void dispatch_events(EventQueue events, void* sender);

private:
	toki::DynamicArray<EventReceiver> m_receivers;
};

}  // namespace toki::platform
