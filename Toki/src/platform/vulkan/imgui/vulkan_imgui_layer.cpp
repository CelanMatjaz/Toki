#include "tkpch.h"
#include "vulkan_imgui_layer.h"
#include "core/engine.h"
#include "events/events.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace Toki {

    void VulkanImGuiLayer::onEvent(Event& e) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) e.setHandled();

        std::cout << "dkwpaojdkawodjawodjaodjawodjv09suf98d\n";

        if (e.getType() == Event::EventType::WindowResized) {
            WindowResizeEvent* ev = (WindowResizeEvent*) &e;
            framebuffer->resize(ev->getWidth(), ev->getHeight());
        }
    }

    void VulkanImGuiLayer::onAttach() {
        FramebufferConfig framebufferConfig{};
        framebufferConfig.target = RenderTarget::Swapchain;
        framebufferConfig.width = 1280;
        framebufferConfig.height = 720;
        framebufferConfig.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        // framebufferConfig.depthAttachment = createRef<Attachment>(Format::Depth, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::DontCare);
        framebufferConfig.colorAttachments = {
            { Format::RGBA8, Samples::Sample1, AttachmentLoadOp::Load, AttachmentStoreOp::Store, RenderTarget::Swapchain, InitialLayout::Present }
        };
        framebuffer = createRef<VulkanFramebuffer>(framebufferConfig);

        std::vector<VkDescriptorPoolSize> poolSizes = {
             { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
             { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
             { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
             { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
             { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
             { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
             { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
             { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        uint32_t maxSets = std::accumulate(poolSizes.begin(), poolSizes.end(), 0, [](int val, const VkDescriptorPoolSize& size) { return val + size.descriptorCount; });

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.maxSets = maxSets;

        vkCreateDescriptorPool(VulkanRenderer::device(), &poolInfo, nullptr, &descriptorPool);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*) Engine::getWindow()->getHandle(), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = VulkanRenderer::instance();
        initInfo.PhysicalDevice = VulkanRenderer::physicalDevice();
        initInfo.Device = VulkanRenderer::device();
        initInfo.QueueFamily = VulkanRenderer::queueFamilyIndexes().graphicsQueueIndex;
        initInfo.Queue = VulkanRenderer::graphicsQueue();
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = descriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = VulkanUtils::getImageCount(VulkanContext::MAX_FRAMES);
        initInfo.ImageCount = VulkanUtils::getImageCount(VulkanContext::MAX_FRAMES);
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&initInfo, framebuffer->getRenderPass());

        {
            VkCommandBuffer commandBuffer = VulkanRenderer::beginSingleUseCommands();
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            VulkanRenderer::endSingleUseCommands(commandBuffer);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void VulkanImGuiLayer::onDetach() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(VulkanRenderer::device(), descriptorPool, nullptr);
    }

    void VulkanImGuiLayer::startImGuiFrame() {
        framebuffer->bind();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VulkanImGuiLayer::endImGuiFrame() {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanRenderer::commandBuffer());

        framebuffer->unbind();
    }

}