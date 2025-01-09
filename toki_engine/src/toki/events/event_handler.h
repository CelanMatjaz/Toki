#pragma once

#include <array>

#include "containers/array_map.h"
#include "core/base.h"
#include "events/event.h"

namespace toki {

struct EventDispatch {
    void* receiver = nullptr;
    EventFunction fn;
};

class EventHandler {
public:
    void bind_event(EventType event_type, void* receiver, EventFunction fn);
    void unbind_event(EventType eventType, void* receiver);
    void dispatch_event(const Event& event, void* sender);

private:
    std::array<array_map<void*, EventDispatch>, (u64) EventType::EVENT_COUNT> m_handlers;
};

}  // namespace toki
