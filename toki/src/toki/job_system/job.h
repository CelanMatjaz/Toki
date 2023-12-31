#pragma once

#include "core/core.h"
#include "future"

namespace Toki {

class JobSystem;

enum class JobStatus {
    Pending,
    InProgress,
    Complete
};

class Job {
    friend JobSystem;

public:
    Job() = default;
    virtual ~Job() = default;

    virtual void execute() = 0;

    void start() { status = JobStatus::InProgress; }

    void end() {
        status = JobStatus::Complete;
        jobCompletedPromise.set_value();
    }

    Ref<Job> getNextJob() { return nextJob; }

    void setNextJob(Ref<Job> job) { nextJob = job; }

protected:
    std::future<void> getFuture() { return jobCompletedPromise.get_future(); }

    JobStatus status = JobStatus::Pending;
    Ref<Job> nextJob;
    std::promise<void> jobCompletedPromise;
};

}  // namespace Toki
