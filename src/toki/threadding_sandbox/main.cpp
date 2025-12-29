// #include <barrier>
// #include <chrono>
// #include <condition_variable>
// #include <mutex>
// #include <print>
// #include <semaphore>
// #include <thread>
// #include <vector>
//
// using namespace std::chrono;
//
// constexpr const uint32_t DELAY_FACTOR		 = 100;
// constexpr const uint32_t WORKER_THREAD_COUNT = 3;
// constexpr const uint32_t STATE_COUNT		 = 2;
//
// std::barrier barrier(4);
// std::binary_semaphore using_state_semaphore{ 1 };
// struct RenderState {
// 	std::mutex lock;
// } worker_states[STATE_COUNT];
// std::atomic<uint32_t> state_index = 0;
//
// void render_thread_func(std::stop_token stop) {
// 	std::println("render thread started");
// 	while (!stop.stop_requested()) {
// 		std::this_thread::sleep_for(16ms * DELAY_FACTOR);  // Work
//
// 		std::println("working on render thread");
//
// 		uint32_t current_state_index = (state_index.load(std::memory_order_acquire) + 1) % STATE_COUNT;
// 		{
// 			std::scoped_lock lock(worker_states[current_state_index].lock);
// 			std::this_thread::sleep_for(2ms * DELAY_FACTOR);  // Set data
// 		}
//
// 		worker_states[current_state_index].is_being_used.release();
// 		state_index.store(current_state_index, std::memory_order_release);
//
// 		barrier.arrive_and_wait();
// 	}
// 	std::println("render thread stopped");
// }
//
// void worker_thread_func(std::stop_token stop, uint32_t i) {
// 	std::println("worker thread {} started", i);
// 	while (!stop.stop_requested()) {
// 		std::println("working on thread {}", i);
// 		uint32_t current_state_index = state_index.load(std::memory_order_acquire);
// 		std::scoped_lock lock(worker_states[current_state_index].lock);
//
// 		using_state_semaphore.acquire();
//
// 		std::this_thread::sleep_for(2ms * DELAY_FACTOR);
//
// 		using_state_semaphore.release();
// 	}
// 	std::println("worker thread {} stopped", i);
// }
//
// int main() {
// 	std::jthread render_thread(render_thread_func);
//
// 	std::vector<std::jthread> workers;
// 	workers.reserve(WORKER_THREAD_COUNT);
// 	for (uint32_t i = 0; i < WORKER_THREAD_COUNT; i++) {
// 		workers.push_back(std::jthread(worker_thread_func, i));
// 	}
//
// 	std::this_thread::sleep_for(60s);
//
// 	std::condition_variable asd;
// 	asd.wait(unique_lock<mutex> &lock)
// }
