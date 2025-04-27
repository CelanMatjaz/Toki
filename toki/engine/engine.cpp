#include "engine.h"

#include <toki/core.h>
// #include <toki/renderer.h>

namespace toki {

Engine::Engine(const Config& config)
//: mEngineAllocator(MB(64))
{
	// pt::initialize(mEngineAllocator);
}

Engine::~Engine() {
	// pt::shutdown(mEngineAllocator);
}

void Engine::run() {
	m_is_running = true;

	Time time;
	Time last_frame_time = time;
	f32 delta_time = 0;
	println("HALO");

	while (m_is_running) {
		Time frame_start_time;
		delta_time = (frame_start_time - last_frame_time).as<Time::Unit::Seconds>() / 1'000;
		last_frame_time = frame_start_time;

		// mRenderer->frame_begin();

		// Submit commands in here

		// mRenderer->frame_end();
	}
}

void Engine::window_add(const char* title, u32 width, u32 height) {}

}  // namespace toki
