#pragma once

#include "job_system/job.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

class CreateShaderModuleJob : public Job {
public:
    static Ref<CreateShaderModuleJob> create(const VulkanContext* context, const std::vector<uint32_t>& code);

    CreateShaderModuleJob() = delete;
    CreateShaderModuleJob(const VulkanContext* context, const std::vector<uint32_t>& code);
    ~CreateShaderModuleJob() = default;

    virtual void execute() override;

    VkShaderModule getShaderModule();

private:
    const VulkanContext* context;
    const std::vector<uint32_t>& code;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
};

}  // namespace Toki
