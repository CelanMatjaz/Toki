#pragma once

#include "job_system/job.h"
#include "mutex"
#include "queue"
#include "vector"
#include "worker_thread.h"

namespace Toki {

class WorkerThread;

class JobSystem {
    friend WorkerThread;

public:
    JobSystem() = delete;
    JobSystem(uint32_t workerCount = 1);
    ~JobSystem();

    std::future<void> queueJob(Ref<Job> job);
    std::future<void> queueJobChain(const std::vector<Ref<Job>> jobs);

    uint32_t getWorkerThreadCount();

private:
    void queueJobs();

    uint32_t workerCount;
    std::vector<Scope<WorkerThread>> workers;

    inline static std::shared_mutex queueLock;
    inline static std::queue<Ref<Job>> queue;
    inline static std::shared_mutex workerLock;
    inline static std::condition_variable_any cv;
    inline static bool notified = false;
};

}  // namespace Toki
