#pragma once

#include <functional>

namespace Toki {

enum class JobPriority {
    Low,
    Normal,
    High
};

struct JobSystemConfig {
    uint32_t maxThreadCount = 0;
};

using JobExecFn = std::function<void()>;
using JobSuccessFn = std::function<void()>;
using JobFailFn = std::function<void()>;

struct JobInfo {
    JobExecFn execFn;
    JobSuccessFn successFn;
    JobFailFn failFn;
};

class Application;

class JobSystem {
    friend Application;

public:
    static JobInfo createJob(JobExecFn execFn, JobSuccessFn successFn, JobFailFn failFn);
    static void queueJob(JobInfo jobInfo, JobPriority priority);
    static void updateQueues();

private:
    static void init(Application* application);
    static void shutdown();
};

}  // namespace Toki
