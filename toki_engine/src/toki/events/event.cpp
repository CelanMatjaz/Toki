#include "event.h"

#include <utility>

#include "input/input.h"

namespace toki {

Event::Event(EventType event_type, EventData data): m_eventType(event_type), m_data(data), m_isHandled(false) {}

EventType Event::get_type() const {
    return m_eventType;
}

EventData Event::get_data() const {
    return m_data;
}

b8 Event::is_handled() const {
    return m_isHandled;
}

void Event::set_handled() {
    m_isHandled = true;
}

Event create_key_event(EventType type, int key, int scancode, KeyboardMods mods) {
    EventData data{};
    data.u16[0] = key;
    data.u16[1] = std::to_underlying(mods);
    data.u32[1] = scancode;
    return Event(type, data);
}

Event create_mouse_button_event(EventType type, int button, KeyboardMods mods, double xpos, double ypos) {
    EventData data{};
    data.u16[0] = button;
    data.u16[1] = (u16) mods;
    data.i16[2] = (u16) xpos;
    data.i16[3] = (u16) ypos;
    return Event(type, data);
}

Event create_mouse_move_event(double xpos, double ypos) {
    return Event(EventType::MouseMove, EventData{ .i32 = { (i32) xpos, (i32) ypos } });
}

Event create_mouse_scroll_event(double xoffset, double yoffset) {
    return Event(EventType::MouseScroll, EventData{ .i32 = { (i32) xoffset, (i32) yoffset } });
}

Event create_window_move_event(double xpos, double ypos) {
    return Event(EventType::WindowMove, EventData{ .i32 = { (i32) xpos, (i32) ypos } });
}

Event create_window_resize_event(int width, int height) {
    return Event(EventType::WindowResize, EventData{ .u32 = { (u32) width, (u32) height } });
}

}  // namespace toki
