#include "event.h"

#include <array>

#include "toki/core/assert.h"
#include "toki/core/core.h"

namespace Toki {

struct EventHandler {
    void* receiver = nullptr;
    EventFunction fn;
};

static std::array<Scope<std::vector<EventHandler>>, (uint32_t) EventType::EVENT_COUNT> handlers;

void Event::bindEvent(EventType eventType, void* receiver, EventFunction fn) {
    uint32_t type = (uint32_t) eventType;

    if (!handlers[type]) {
        handlers[type] = createScope<std::vector<EventHandler>>();
    }

    for (uint32_t i = 0; i < handlers[type]->size(); ++i) {
        TK_ASSERT(handlers[type]->operator[](i).receiver != receiver, "Duplicate receiver");
    }

    handlers[type]->emplace_back(EventHandler{ receiver, fn });
}

void Event::unbindEvent(EventType eventType, void* receiver) {
    uint32_t type = (uint32_t) eventType;

    for (uint32_t i = 0; i < handlers[type]->size(); ++i) {
        if (handlers[type]->operator[](i).receiver == receiver) {
            handlers[type]->erase(handlers[type]->begin() + i);
            return;
        }
    }

    TK_ASSERT(false, "No event bound for receiver");
}

void Event::dispatchEvent(const Event& event, void* sender) {
    uint32_t type = (uint32_t) event.getType();

    TK_ASSERT((bool) handlers[type] && handlers[type]->size() > 0, "No events registered for handler");

    for (uint32_t i = 0; i < handlers[type]->size(); ++i) {
        auto& handler = handlers[type]->operator[](i);
        handler.fn(sender, handler.receiver, event);
    }
}

Event::Event(EventType eventType, EventData data) : m_eventType(eventType), m_data(data) {}

EventType Event::getType() const {
    return m_eventType;
}

EventData Event::getData() const {
    return m_data;
}

bool Event::isHandled() const {
    return m_isHandled;
}

void Event::setHandled() {
    m_isHandled = true;
}

}  // namespace Toki
