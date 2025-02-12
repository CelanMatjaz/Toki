#pragma once

#include <array>
#include <utility>

#include "events/event.h"

namespace toki {

struct EventDispatch {
    void* receiver = nullptr;
    EventFunction fn;
};

class EventHandler {
public:
    void bind_all(void* receiver, EventFunction fn);
    void bind_event(EventType event_type, void* receiver, EventFunction fn);
    void unbind_event(EventType eventType, void* receiver = nullptr);
    void dispatch_event(Event& event, void* sender = nullptr);

private:
    std::array<std::vector<EventDispatch>, std::to_underlying(EventType::EVENT_COUNT)> m_listeners;
    std::vector<EventDispatch> m_allEventListeners;
};

}  // namespace toki
