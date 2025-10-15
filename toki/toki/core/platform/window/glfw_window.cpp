#include <toki/core/types.h>

#include "toki/core/platform/window/window.h"

#if defined(TK_WINDOW_SYSTEM_GLFW)

	#include <GLFW/glfw3.h>

namespace toki {

	#define HANDLE reinterpret_cast<GLFWwindow*>(m_handle)

void window_system_initialize([[maybe_unused]] const WindowSystemConfig& config) {
	glfwInit();
}

void window_system_shutdown() {
	glfwTerminate();
}

void window_system_poll_events() {
	glfwPollEvents();
}

struct StaticWindowFunctions {
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};

Window::Window(const WindowConfig& config) {
	TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, config.flags & WINDOW_FLAG_RESIZABLE);

	GLFWwindow* window = glfwCreateWindow(
		static_cast<i32>(config.width), static_cast<i32>(config.height), config.title, nullptr, nullptr);
	TK_ASSERT(window != nullptr, "Window was not created");
	m_handle = window;

	if (config.flags & WINDOW_FLAG_SHOW_ON_CREATE) {
		glfwShowWindow(window);
	}

	glfwSetCursorPosCallback(HANDLE, StaticWindowFunctions::cursor_position_callback);
	glfwSetMouseButtonCallback(HANDLE, StaticWindowFunctions::mouse_button_callback);

	glfwSetWindowUserPointer(window, this);
}

Window::~Window() {
	glfwDestroyWindow(HANDLE);
}

b8 Window::should_close() const {
	return glfwWindowShouldClose(HANDLE) == GLFW_TRUE;
}

Vector2u32 Window::get_dimensions() const {
	int width, height;
	glfwGetFramebufferSize(HANDLE, &width, &height);
	return Vector2u32{ static_cast<u32>(width), static_cast<u32>(height) };
}

void StaticWindowFunctions::cursor_position_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	toki::Window* w = reinterpret_cast<toki::Window*>(glfwGetWindowUserPointer(window));
	w->m_eventQueue.emplace_back(
		EventType::MOUSE_MOVE, EventData{ .window = { .x = static_cast<i16>(xpos), .y = static_cast<i16>(ypos) } });
}
void StaticWindowFunctions::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	toki::Window* w = reinterpret_cast<toki::Window*>(glfwGetWindowUserPointer(window));

	EventType type;
	switch (action) {
		case GLFW_PRESS:
			type = EventType::MOUSE_PRESS;
			break;
		case GLFW_RELEASE:
			type = EventType::MOUSE_RELEASE;
			break;
		case GLFW_REPEAT:
			type = EventType::MOUSE_REPEAT;
			break;
		default:
			TK_UNREACHABLE();
	}

	w->m_eventQueue.emplace_back(type, EventData{ .mouse = { .x = 0, .y = 0, .button = static_cast<u8>(button) } });
}

}  // namespace toki

#endif
