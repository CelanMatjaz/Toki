#include "worker_thread.h"

namespace Toki {

WorkerThread::WorkerThread(uint32_t workerId) : workerId(workerId), worker(workerFunction, this) {}

WorkerThread::~WorkerThread() {
    worker.join();
}

void WorkerThread::workerFunction(WorkerThread* worker) {
    while (running) {
        std::shared_lock<std::shared_mutex> wlck(JobSystem::workerLock);

        WorkerThread::jobSystem->cv.wait(wlck);

        if (JobSystem::queue.empty()) {
            continue;
        }

        Ref<Job> currentJob;

        {
            std::scoped_lock lck(JobSystem::queueLock);
            currentJob = JobSystem::queue.front();
            JobSystem::queue.pop();
        }

        while (currentJob) {
            currentJob->start();
            currentJob->execute();
            currentJob->end();
            currentJob = currentJob->getNextJob();
        }

        if (WorkerThread::jobSystem) {
            WorkerThread::jobSystem->queueJobs();
        }
    }
}

}  // namespace Toki
