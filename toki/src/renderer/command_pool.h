#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <vector>

namespace Toki {

// State explanations
// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#commandbuffers-lifecycle
enum class CommandBufferState {
    Initial,
    Recording,
    Executable,
    Pending,
    // Invalid
};

struct CommandBuffer {
    CommandBuffer(VkCommandBuffer commandBuffer) : m_commandBuffer(commandBuffer){};
    CommandBuffer(CommandBuffer&&) = default;
    CommandBuffer(const CommandBuffer&) = default;

    void beginRecording(bool isSingleUse, bool isRenderPassContinue, bool isSimultaneousUse);
    void endRecording();
    void reset();

    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    CommandBufferState m_state = CommandBufferState::Initial;
    bool m_didRecordCommands = false;
    operator VkCommandBuffer() const { return m_commandBuffer; }
};

class CommandPool {
public:
    CommandPool();
    ~CommandPool();

    CommandBuffer& allocateCommandBuffer();
    CommandBuffer& getCommandBuffer();
    void beginCommandBuffers();
    void endCommandBuffers();
    void resetCommandBuffers();
    void setSubmittableCommandBuffers();

    const std::vector<VkCommandBuffer>& getSubmittableCommandBuffers() const;

    static void submitSingleUseCommands(VkQueue queue, std::function<void(VkCommandBuffer cmd)> fn);

    operator VkCommandPool() const { return m_commandPool; };

private:
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<CommandBuffer> m_allocatedCommandBuffers;
    std::vector<VkCommandBuffer> m_submittableCommandBuffers;
};

}  // namespace Toki
