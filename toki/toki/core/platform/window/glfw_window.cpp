#include <toki/core/platform/window/glfw_defines.h>
#include <toki/core/platform/window/window.h>
#include <toki/core/types.h>

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
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
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
	glfwSetKeyCallback(HANDLE, StaticWindowFunctions::key_callback);

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
	w->m_input.mouse_delta.x = w->m_input.mouse_position.x - xpos;
	w->m_input.mouse_delta.y = w->m_input.mouse_position.y - ypos;
	w->m_input.mouse_position.x = xpos;
	w->m_input.mouse_position.y = ypos;
}

static void handle_mods(Mods& mods, i32 button, i32 action);

void StaticWindowFunctions::mouse_button_callback(
	GLFWwindow* window, i32 button, i32 action, [[maybe_unused]] i32 mods) {
	toki::Window* w = reinterpret_cast<toki::Window*>(glfwGetWindowUserPointer(window));

	EventType type;
	switch (action) {
		case GLFW_PRESS:
			type = EventType::MOUSE_PRESS;
			w->m_input.mouse_buttons.set(button, true);
			handle_mods(w->m_input.mods, button, action);
			break;
		case GLFW_RELEASE:
			type = EventType::MOUSE_RELEASE;
			w->m_input.mouse_buttons.set(button, false);
			handle_mods(w->m_input.mods, button, action);
			break;
		case GLFW_REPEAT:
			type = EventType::MOUSE_REPEAT;
			break;
		default:
			TK_UNREACHABLE();
	}

	w->m_input.event_queue.emplace_back(
		type, EventData{ .mouse = { .x = 0, .y = 0, .button = static_cast<u8>(button) } });
}

void StaticWindowFunctions::key_callback(
	GLFWwindow* window, int key, int scancode, int action, [[maybe_unused]] int mods) {
	toki::Window* w = reinterpret_cast<toki::Window*>(glfwGetWindowUserPointer(window));

	Key mapped_key = map_glfw_key(key);

	EventType type;
	switch (action) {
		case GLFW_PRESS:
			type = EventType::KEY_PRESS;
			w->m_input.keys.set(static_cast<u32>(mapped_key), true);
			break;
		case GLFW_RELEASE:
			type = EventType::KEY_RELEASE;
			w->m_input.keys.set(static_cast<u32>(mapped_key), false);
			break;
		case GLFW_REPEAT:
			type = EventType::KEY_REPEAT;
			break;
		default:
			TK_UNREACHABLE();
	}

	w->m_input.event_queue.emplace_back(
		type, EventData{ .key = { .scan = static_cast<u32>(scancode), .key = mapped_key } });
}

static void handle_mods(Mods& mods, i32 button, i32 action) {
	auto set_mod = [](Mods& mods_, i32 action_, i16 mod) {
		if (action_ == GLFW_PRESS) {
			mods_.mods |= mod;
		} else if (action_ == GLFW_REPEAT) {
			mods_.mods &= ~mod;
		}
	};

	switch (button) {
		case GLFW_KEY_LEFT_SHIFT:
			set_mod(mods, action, MOD_BITS_LSHIFT);
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			set_mod(mods, action, MOD_BITS_RSHIFT);
			break;
		case GLFW_KEY_LEFT_CONTROL:
			set_mod(mods, action, MOD_BITS_LCONTROL);
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			set_mod(mods, action, MOD_BITS_RCONTROL);
			break;
		case GLFW_KEY_LEFT_ALT:
			set_mod(mods, action, MOD_BITS_LALT);
			break;
		case GLFW_KEY_RIGHT_ALT:
			set_mod(mods, action, MOD_BITS_RALT);
			break;
		case GLFW_KEY_LEFT_SUPER:
			set_mod(mods, action, MOD_BITS_LSUPER);
			break;
		case GLFW_KEY_RIGHT_SUPER:
			set_mod(mods, action, MOD_BITS_RSUPER);
			break;
		default:
			break;
	}
}

}  // namespace toki

#endif
