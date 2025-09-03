#include <toki/core/core.h>

#include "toki/platform/window.h"

int main() {
	toki::window_system_initialize();

	toki::WindowConfig window_config{};
	window_config.title = "Test";
	window_config.width = 400;
	window_config.height = 400;
	window_config.flags = toki::SHOW_ON_CREATE;
	toki::Window window(window_config);

	while (!window.should_close()) {
		toki::print("test");
		toki::window_system_poll_events();
	}

	toki::window_system_shutdown();
}
