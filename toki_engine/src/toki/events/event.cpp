#include "event.h"

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

Event create_key_event(EventType type, int key, int scancode, int action, int mods) {
    return Event(type, EventData{ .u16 = { (u16) key, (u16) scancode, (u16) action, (u16) mods } });
}

Event create_mouse_button_event(EventType type, int button, int action, int mods) {
    return Event(type, EventData{ .u16 = { (u16) button, (u16) action, (u16) mods } });
}

Event create_mouse_move_event(double xpos, double ypos) {
    return Event(EventType::MouseMove, EventData{ .f32 = { (f32) xpos, (f32) ypos } });
}

Event create_mouse_scroll_event(double xoffset, double yoffset) {
    return Event(EventType::MouseScroll, EventData{ .f32 = { (f32) xoffset, (f32) yoffset } });
}

Event create_window_move_event(double xpos, double ypos) {
    return Event(EventType::WindowMove, EventData{ .f32 = { (f32) xpos, (f32) ypos } });
}

Event create_window_resize_event(int width, int height) {
    return Event(EventType::WindowMove, EventData{ .u32 = { (u32) width, (u32) height } });
}

}  // namespace toki
