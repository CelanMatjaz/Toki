#include <GLFW/glfw3.h>

#include "../platform/attributes.h"
#include "../platform/defines.h"
#include "../window/window.h"

namespace toki {

#define USER_PTR(window) reinterpret_cast<Window*>(glfwGetWindowUserPointer(window))

#define QUEUE_EVENT(event)          \
	Window* ptr = USER_PTR(window); \
	ptr->handler.queue().add_event(event);

void window_system_initialize() {
	glfwInit();
}

void window_system_shutdown() {
	glfwTerminate();
}

void Window::poll_events() {
	glfwPollEvents();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	WindowEvent type = WindowEvent::KEY_PRESS;

	switch (action) {
		case GLFW_RELEASE:
			type = WindowEvent::KEY_RELEASE;
			break;
		case GLFW_PRESS:
			type = WindowEvent::KEY_PRESS;
			break;
		case GLFW_REPEAT:
			type = WindowEvent::KEY_REPEAT;
			break;
		default:
			TK_UNREACHABLE();
	}

	EventData data = { .key = {} };
	QUEUE_EVENT(Event::create(type, data));
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	WindowEvent type = WindowEvent::MOUSE_PRESS;

	switch (action) {
		case GLFW_PRESS:
			type = WindowEvent::MOUSE_PRESS;
			break;
		case GLFW_RELEASE:
			type = WindowEvent::MOUSE_RELEASE;
			break;
		default:
			TK_UNREACHABLE();
	}

	EventData data = { .key = {} };
	QUEUE_EVENT(Event::create(type, data));
}

static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	EventData data = { 
		.mouse_move = {
			.x = static_cast<i32>(xpos),
			.y = static_cast<i32>(ypos),
		}, 
	};

	QUEUE_EVENT(Event::create(WindowEvent::MOUSE_MOVE, data));
}

static void mouse_enter_callback(GLFWwindow* window, int entered) {
	QUEUE_EVENT(Event::create(WindowEvent::WINDOW_ENTER));
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	EventData data = { 
		.mouse_move = {
			.x = static_cast<i32>(xoffset),
			.y = static_cast<i32>(yoffset),
		}, 
	};

	QUEUE_EVENT(Event::create(WindowEvent::MOUSE_SCROLL, data));
}

static void window_close_callback(GLFWwindow* window) {
	QUEUE_EVENT(Event::create(WindowEvent::WINDOW_CLOSE));
}

static void window_move_callback(GLFWwindow* window, int xpos, int ypos) {
	EventData data = { 
		.window_move = {
			.x = static_cast<i32>(xpos),
			.y = static_cast<i32>(ypos),
		}, 
	};

	QUEUE_EVENT(Event::create(WindowEvent::WINDOW_MOVE, data));
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
	EventData data = { 
		.window_resize = {
			.x = static_cast<u32>(width),
			.y = static_cast<u32>(height),
		}, 
	};
	QUEUE_EVENT(Event::create(WindowEvent::WINDOW_RESIZE, data));
}

static void window_maximize_callback(GLFWwindow* window, int maximized) {
	QUEUE_EVENT(Event::create(maximized ? WindowEvent::WINDOW_MAXIMIZE : WindowEvent::WINDOW_RESTORE));
}

static void window_iconify_callback(GLFWwindow* window, int iconified) {
	QUEUE_EVENT(Event::create(iconified ? WindowEvent::WINDOW_MINIMIZE : WindowEvent::WINDOW_RESTORE));
}

static void window_focus_callback(GLFWwindow* window, int focused) {
	QUEUE_EVENT(Event::create(WindowEvent::WINDOW_FOCUS));
}

Window::Window(const Config& config) {
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
	this->m_native_window = { window };
	glfwSetWindowUserPointer(window, this);

	// Window callbacks
	glfwSetKeyCallback(window, key_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetCursorEnterCallback(window, mouse_enter_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwSetWindowPosCallback(window, window_move_callback);
	glfwSetWindowSizeCallback(window, window_resize_callback);
	glfwSetFramebufferSizeCallback(window, window_resize_callback);
	glfwSetWindowMaximizeCallback(window, window_maximize_callback);
	glfwSetWindowFocusCallback(window, window_focus_callback);
}

Window::~Window() {
	glfwDestroyWindow(m_native_window.window);
}

}  // namespace toki
