#pragma once

#include "core/core.h"
#include "job.h"
#include "job_system.h"
#include "shared_mutex"

namespace Toki {

class WorkerThread {
    friend JobSystem;

public:
    WorkerThread() = delete;
    WorkerThread(uint32_t workerId);
    ~WorkerThread();

private:
    uint32_t workerId;
    std::thread worker;

    inline static bool running = true;
    inline static JobSystem* jobSystem = nullptr;

    static void workerFunction(WorkerThread* worker);
};

}  // namespace Toki
