#pragma once

#include "filesystem"
#include "job_system/job.h"
#include "renderer/renderer_types.h"

namespace Toki {

class CompileShaderJob : public Job {
public:
    static Ref<CompileShaderJob> create(std::filesystem::path path);

    CompileShaderJob() = delete;
    CompileShaderJob(std::filesystem::path path);
    virtual ~CompileShaderJob();

    virtual void execute() override;

    auto getCode() { return data; }

private:
    std::filesystem::path path;
    std::unordered_map<ShaderStage, std::vector<uint32_t>> data;
};

}  // namespace Toki
