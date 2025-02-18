#include "event_handler.h"

#include <algorithm>
#include <utility>

namespace toki {

void EventHandler::bind_all(void* receiver, EventFunction fn) {
    m_allEventListeners.emplace_back(receiver, fn);
}

void EventHandler::bind_event(EventType event_type, void* receiver, EventFunction fn) {
    u32 index = std::to_underlying(event_type);
    m_listeners[index].emplace_back(receiver, fn);
}

void EventHandler::unbind_event(EventType event_type, void* receiver) {
    u32 index = std::to_underlying(event_type);
    auto listener =
        std::ranges::find_if(m_listeners[index].begin(), m_listeners[index].end(), [receiver](EventDispatch dispatch) {
            return dispatch.receiver == receiver;
        });
    if (listener != m_listeners[index].end()) {
        m_listeners[index].erase(listener);
    }
}

void EventHandler::dispatch_event(Event& event, void* sender) {
    u32 index = std::to_underlying(event.get_type());

    for (auto& dispatch : m_allEventListeners) {
        dispatch.fn(sender, dispatch.receiver, event);
    }

    for (auto& dispatch : m_listeners[index]) {
        dispatch.fn(sender, dispatch.receiver, event);
    }
}

}  // namespace toki
