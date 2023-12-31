#include "create_shader_module_job.h"

#include "core/assert.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

Ref<CreateShaderModuleJob> CreateShaderModuleJob::create(const VulkanContext* context, const std::vector<uint32_t>& code) {
    return createRef<CreateShaderModuleJob>(context, code);
}

CreateShaderModuleJob::CreateShaderModuleJob(const VulkanContext* context, const std::vector<uint32_t>& code) : context(context), code(code) {}

void CreateShaderModuleJob::execute() {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size() * 4;
    shaderModuleCreateInfo.pCode = code.data();

    TK_ASSERT_VK_RESULT(
        vkCreateShaderModule(context->device, &shaderModuleCreateInfo, context->allocationCallbacks, &shaderModule), "Could not create shader module"
    );
}

VkShaderModule CreateShaderModuleJob::getShaderModule() {
    return shaderModule;
}

}  // namespace Toki
