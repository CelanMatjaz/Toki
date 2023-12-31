#include "events.h"

#include "core/assert.h"

namespace Toki {

Event::Event(EventType eventType, uint64_t data) : eventType(eventType), data(data) {}

EventType Event::getType() const {
    return eventType;
}

int16_t Event::getData(uint8_t word) const {
    TK_ASSERT(word < 4, "Word index is not between 0-3");
    return (int16_t) (data >> (word * 16));
}

bool Event::getIsHandled() const {
    return isHandled;
}

void Event::handle() {
    isHandled = true;
}

}  // namespace Toki
