#include <toki/core/common/assert.h>
#include <toki/platform/window.h>

#if defined(TK_WINDOW_SYSTEM_GLFW)

	#include <GLFW/glfw3.h>

namespace toki::platform {

	#define HANDLE reinterpret_cast<GLFWwindow*>(m_handle)

void window_system_initialize(const WindowSystemConfig& config) {
	glfwInit();
}

void window_system_shutdown() {
	glfwTerminate();
}

void window_system_poll_events() {
	glfwPollEvents();
}

Window::Window(const WindowConfig& config) {
	TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

	++s_windowCount;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, config.flags & RESIZABLE);

	GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
	TK_ASSERT(window != nullptr, "Window was not created");
	m_handle = window;

	if (config.flags & SHOW_ON_CREATE) {
		glfwShowWindow(window);
	}

	glfwSetWindowUserPointer(window, this);
}

Window::~Window() {
	--s_windowCount;

	glfwDestroyWindow(HANDLE);
}

b8 Window::should_close() const {
	return glfwWindowShouldClose(HANDLE) == GLFW_TRUE;
}

void Window::set_renderer_pointer(void* ptr) {
	m_rendererData = ptr;
}

void* Window::get_renderer_pointer() const {
	return m_rendererData;
}

Vec2u32 Window::get_dimensions() const {
	int width, height;
	glfwGetFramebufferSize(HANDLE, &width, &height);
	return Vec2u32{ static_cast<u32>(width), static_cast<u32>(height) };
}

}  // namespace toki::platform

#endif
