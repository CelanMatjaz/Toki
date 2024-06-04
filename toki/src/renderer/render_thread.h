#pragma once

#include <barrier>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "renderer/renderer_state.h"
#include "toki/renderer/command.h"

namespace Toki {

class RenderThread {
public:
    RenderThread();
    ~RenderThread();

    static void submitWork(SubmitFunction submitFn);
    static void startWork();
    static void startThreads(uint32_t threadCount);
    static void stopThreads();
    static void waitForThreads();

    static void beginRecordingCommands();
    static void endRecordingCommands();
    static std::vector<VkCommandBuffer> getCommandBuffersForSubmission();

private:
    struct RenderThreadState {
        std::array<CommandPool, MAX_FRAMES> commandPools;
        std::queue<SubmitFunction> workQueue{};
        std::mutex lock;
        std::condition_variable cv{};

        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

        TimePoint lastSubmissionStartTime;
        TimePoint lastSubmissionEndTime;

        uint32_t submissionCount = 0;
        uint32_t lastSubmitssionCount = 0;
    } m_state;

    std::jthread m_thread;

    static void renderThreadFunc(std::stop_token stopToken, RenderThreadState* state);
    inline static std::vector<Ref<RenderThread>> s_renderThreads;
    inline static Scope<std::barrier<>> s_barrier;
};

}  // namespace Toki
