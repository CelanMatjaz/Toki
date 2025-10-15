#include "toki/core/platform/window/window.h"

namespace toki {

void Window::set_renderer_pointer(void* ptr) {
	m_rendererData = ptr;
}

void* Window::get_renderer_pointer() const {
	return m_rendererData;
}

bool Window::poll_event(Event& event) {
	return m_eventQueue.pop(event);
}

}  // namespace toki
