#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <vector>

namespace Toki {

class CommandPool {
public:
    CommandPool(uint32_t initialCommandBufferCount = 1);
    ~CommandPool();

    VkCommandBuffer getNewCommandBuffer();
    void endCommandBuffers();
    void resetCommandBuffers();
    const std::vector<VkCommandBuffer> getSubmittableCommandBuffers() const;

    static void submitSingleUseCommands(VkQueue queue, std::function<void(VkCommandBuffer cmd)> fn);

    operator VkCommandPool() const { return m_commandPool; };

private:
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_allocatedCommandBuffers;
    uint32_t m_inUseCommandBufferCount = 0;
};

}  // namespace Toki
