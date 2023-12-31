#include "job_system.h"

#include "core/core.h"
#include "mutex"

namespace Toki {

JobSystem::JobSystem(uint32_t workerCount) : workerCount(workerCount) {
    for (uint32_t i = 0; i < workerCount; ++i) {
        workers.emplace_back(createScope<WorkerThread>(i));
    }

    WorkerThread::jobSystem = this;
}

JobSystem::~JobSystem() {
    while (!queue.empty()) {
        queueJobs();
    }
    WorkerThread::running = false;
    cv.notify_all();
    workers.clear();
}

uint32_t JobSystem::getWorkerThreadCount() {
    return workerCount;
}

std::future<void> JobSystem::queueJob(Ref<Job> job) {
    {
        std::scoped_lock qlck(queueLock);
        queue.push(job);
    }
    queueJobs();
    return job->getFuture();
}

std::future<void> JobSystem::queueJobChain(const std::vector<Ref<Job>> jobs) {
    for (uint32_t i = 0; i < jobs.size() - 1; ++i) {
        jobs[i]->setNextJob(jobs[i + 1]);
    }

    {
        std::scoped_lock qlck(queueLock);
        queue.push(jobs.front());
    }

    queueJobs();
    return jobs.back()->getFuture();
}

void JobSystem::queueJobs() {
    if (queue.size() == 0) return;
    std::shared_lock lck(workerLock);
    cv.notify_one();
}

}  // namespace Toki
