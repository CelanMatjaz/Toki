#include "event_handler.h"

#include "core/assert.h"

namespace toki {

void EventHandler::bind_event(EventType event_type, void* receiver, EventFunction fn) {
    u32 index = (u32) event_type;
    TK_ASSERT(!m_handlers[index].contains(receiver), "Event already bound for this reciever");
    m_handlers[index].emplace(receiver, EventDispatch{ receiver, fn });
}

void EventHandler::unbind_event(EventType event_type, void* receiver) {
    u32 index = (u32) event_type;
    TK_ASSERT(m_handlers[index].contains(receiver), "Reciever not bound for event");
    m_handlers[index].erase(receiver);
}

void EventHandler::dispatch_event(const Event& event, void* sender) {
    u32 index = (u32) event.get_type();
    for (auto& [_, handler] : m_handlers[index]) {
        handler.fn(sender, handler.receiver, event);
    }
}

}  // namespace toki
