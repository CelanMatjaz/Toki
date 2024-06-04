#include "render_thread.h"

#include "renderer/vulkan_command.h"
#include "toki/core/logging.h"

namespace Toki {

RenderThread::RenderThread() : m_state(RenderThreadState{}), m_thread(std::jthread(renderThreadFunc, &m_state)) {}

RenderThread::~RenderThread() {
    m_thread.request_stop();
    m_state.cv.notify_all();
    m_thread.join();
}

void RenderThread::submitWork(SubmitFunction submitFn) {
    static auto queueWork = [](RenderThreadState& state, SubmitFunction submitFn) {
        std::scoped_lock lck(state.lock);
        state.workQueue.push(submitFn);
    };

    // Find free thread
    for (auto it = s_renderThreads.rbegin(); it != s_renderThreads.rend(); ++it) {
        if (it != s_renderThreads.rbegin() && (*it)->m_state.workQueue.size() > (*(it - 1))->m_state.workQueue.size()) {
            auto threadData = (*(it - 1));
            queueWork(threadData->m_state, submitFn);
            return;
        }
    }

    // Add work to first thread if no next thread is free
    queueWork(s_renderThreads[0]->m_state, submitFn);
}

void RenderThread::startWork() {
    for (auto it = s_renderThreads.begin(); it != s_renderThreads.end(); ++it) {
        (*it)->m_state.cv.notify_all();
    }
}

void RenderThread::startThreads(uint32_t threadCount) {
    LOG_INFO("Creating {} render thread(s)", threadCount);

    s_barrier = createScope<std::barrier<>>(threadCount + 1);

    for (uint32_t i = 0; i < threadCount; ++i) {
        s_renderThreads.emplace_back(createRef<RenderThread>());
    }
}

void RenderThread::stopThreads() {
    LOG_INFO("Stopping render {} thread(s)", s_renderThreads.size());

    for (auto& thread : s_renderThreads) {
        thread->m_thread.request_stop();
        thread->m_state.cv.notify_all();
    }

    s_barrier->arrive_and_wait();

    s_renderThreads.clear();
}

void RenderThread::waitForThreads() {
    s_barrier->arrive_and_wait();
}

void RenderThread::beginRecordingCommands() {
    auto now = std::chrono::high_resolution_clock::now();
    for (auto it = s_renderThreads.begin(); it != s_renderThreads.end(); ++it) {
        RenderThreadState& state = (*it)->m_state;
        state.lastSubmitssionCount = state.submissionCount;
        state.submissionCount = 0;
        state.lastSubmissionStartTime = now;
        state.lastSubmissionEndTime = now;
        state.commandPools[s_currentFrameIndex].resetCommandBuffers();
    }
}

void RenderThread::endRecordingCommands() {
    for (auto it = s_renderThreads.begin(); it != s_renderThreads.end(); ++it) {
        RenderThreadState& state = (*it)->m_state;
        state.commandPools[s_currentFrameIndex].endCommandBuffers();
    }
}

std::vector<VkCommandBuffer> RenderThread::getCommandBuffersForSubmission() {
    std::vector<VkCommandBuffer> commandBuffers;

    for (auto it = s_renderThreads.begin(); it != s_renderThreads.end(); ++it) {
        commandBuffers.append_range((*it)->m_state.commandPools[s_currentFrameIndex].getSubmittableCommandBuffers());
    }

    return commandBuffers;
}

void RenderThread::renderThreadFunc(std::stop_token stopToken, RenderThreadState* state) {
    // LOG_INFO("Starting render thread {}", std::this_thread::get_id());

    while (!stopToken.stop_requested()) {
        std::unique_lock lock(state->lock);
        state->cv.wait(lock);

        while (!state->workQueue.empty()) {
            SubmitFunction work = state->workQueue.front();
            ++state->submissionCount;

            VkCommandBuffer commandBuffer = state->commandPools[s_currentFrameIndex].getNewCommandBuffer();
            VulkanCommand command(commandBuffer);
            state->workQueue.front()(command);

            state->lastSubmissionEndTime = std::chrono::high_resolution_clock::now();
            state->workQueue.pop();
        }

        s_barrier->arrive_and_wait();
    }

    // LOG_INFO("Stopping render thread {}", std::this_thread::get_id());
}

}  // namespace Toki
