#include "job_system.h"

#include <algorithm>
#include <condition_variable>
#include <queue>
#include <shared_mutex>
#include <stop_token>
#include <thread>
#include <vector>

#include "toki/core/assert.h"
#include "toki/core/core.h"

namespace Toki {

class JobWorker;

struct JobSystemState {
    bool initialized = true;

    Application* application;

    std::queue<JobInfo> queues[3];

    std::vector<Ref<JobWorker>> jobWorkers;
};

static JobSystemState* state = nullptr;

class JobWorker {
public:
    JobWorker() : m_workerThread(workerThreadFunc, this) {}
    ~JobWorker() { m_workerThread.join(); }

    void startJob(JobInfo job) {
        std::lock_guard lck(m_lock);
        free = false;
        m_currentJob = job;
        m_cv.notify_all();
    }

    bool free = true;

    static void stopWorkers() {
        s_running = false;

        for (auto& w : state->jobWorkers) {
            std::lock_guard lck(w->m_lock);
            w->free = false;
            w->m_cv.notify_all();
        }
    }

protected:
    std::jthread m_workerThread;
    JobInfo m_currentJob;
    std::mutex m_lock;
    std::condition_variable m_cv;

    inline static bool s_running = true;

    static void workerThreadFunc(std::stop_token stopToken, JobWorker* t) {
        do {
            std::unique_lock lck(t->m_lock);
            t->free = true;
            t->m_cv.wait(lck, [t]() { return !t->free; });

            if (stopToken.stop_requested() || !s_running) return;

            try {
                t->m_currentJob.execFn();
                t->m_currentJob.successFn();
            } catch (...) {
                t->m_currentJob.failFn();
            }
        } while (s_running && !stopToken.stop_requested());
    }
};

void JobSystem::init(Application* application) {
    TK_ASSERT(state == nullptr, "Job system state is already initialized");
    TK_ASSERT(application != nullptr, "Cannot init Job system without a provided Application pointer");

    state = new JobSystemState();
    state->application = application;

    for (uint32_t i = 0; i < 5; ++i) {
        state->jobWorkers.emplace_back(createRef<JobWorker>());
    }
}

void JobSystem::shutdown() {
    TK_ASSERT(state != nullptr && state->initialized, "Job system state is not initialized");

    JobWorker::stopWorkers();
    state->jobWorkers.clear();

    delete state;
}

JobInfo JobSystem::createJob(JobExecFn execFn, JobSuccessFn successFn, JobFailFn failFn) {
    return JobInfo{ execFn, successFn, failFn };
}

void JobSystem::queueJob(JobInfo jobInfo, JobPriority priority) {
    switch (priority) {
        case JobPriority::High:
            state->queues[0].push(jobInfo);
            break;
        case JobPriority::Normal:
            state->queues[1].push(jobInfo);
            break;
        case JobPriority::Low:
            state->queues[2].push(jobInfo);
            break;
        default:
            std::unreachable();
    }

    updateQueues();
}

static std::mutex queueLock;

void JobSystem::updateQueues() {
    TK_ASSERT(state != nullptr, "Job system state has to be initialized");

    if (!state->initialized) return;

    for (auto it = state->jobWorkers.begin(); it != state->jobWorkers.end(); ++it) {
        if (!(*it)->free) {
            continue;
        }

        for (uint8_t i = 0; i < 3; ++i) {
            if (state->queues[i].size() == 0) {
                continue;
            }

            std::scoped_lock lck(queueLock);
            (*it)->startJob(state->queues[i].front());
            state->queues[i].pop();
        }
    }
}

}  // namespace Toki
