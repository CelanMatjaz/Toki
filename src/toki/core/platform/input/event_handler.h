#pragma once

#include <toki/core/containers/arena.h>
#include <toki/core/containers/dynamic_array.h>
#include <toki/core/containers/ring_buffer.h>
#include <toki/core/platform/input/event.h>

namespace toki {

constexpr u32 EVENT_ARENA_QUEUE_ELEMENT_COUNT = 1024;
using EventQueue = toki::RingBuffer<Event, EVENT_ARENA_QUEUE_ELEMENT_COUNT>;

using EventFunction = void (&)(void* sender, void* listener, const Event& event);

struct EventListener {
	void* listener{};
	EventFunction fn;
};

class EventHandler {
public:
	EventHandler() = default;

	void register_listener(void* listener, EventFunction fn);
	void unregister_listener(void* listener);
	void dispatch_event(const Event& event, void* sender);

private:
	toki::DynamicArray<EventListener> m_listeners;
};

}  // namespace toki
