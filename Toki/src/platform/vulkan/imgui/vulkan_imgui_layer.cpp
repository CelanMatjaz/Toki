#include "tkpch.h"
#include "vulkan_imgui_layer.h"
#include "core/engine.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"

#include "backends/imgui_impl_vulkan.h"

#ifdef TK_WIN32
#include "backends/imgui_impl_win32.h"
#endif

#ifdef TK_GLFW
#include "backends/imgui_impl_glfw.h"
#endif


namespace Toki {

    void VulkanImGuiLayer::onEvent(Event& e) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) e.setHandled();
    }

    void VulkanImGuiLayer::onAttach() {
        RenderPassConfig renderPassConfig{};
        renderPassConfig.nSubpasses = 1;
        renderPassConfig.attachments = {
            { Format::RGBA8, Samples::Sample1, AttachmentLoadOp::Load, AttachmentStoreOp::Store, RenderTarget::Swapchain, false, InitialLayout::Present }
        };
        renderPass = RenderPass::create(renderPassConfig);

        FramebufferConfig framebufferConfig{};
        framebufferConfig.renderPass = renderPass;
        framebufferConfig.width = Engine::getWindow()->getWidth();
        framebufferConfig.height = Engine::getWindow()->getHeight();
        framebufferConfig.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        framebuffer = Framebuffer::create(framebufferConfig);

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

#ifdef TK_WIN32
        ImGui_ImplWin32_Init((HWND) Engine::getWindow()->getHandle());
#endif

#ifdef TK_GLFW
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*) Engine::getWindow()->getHandle(), true);
#endif

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

        ImGui_ImplVulkan_Init(&initInfo, ((VulkanRenderPass*) renderPass.get())->getHandle());

        {
            VkCommandBuffer commandBuffer = VulkanRenderer::beginSingleUseCommands();
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            VulkanRenderer::endSingleUseCommands(commandBuffer);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void VulkanImGuiLayer::onDetach() {
        ImGui_ImplVulkan_Shutdown();

#ifdef TK_WIN32
        ImGui_ImplWin32_Shutdown();
#endif

#ifdef TK_GLFW
        ImGui_ImplGlfw_Shutdown();
#endif

        ImGui::DestroyContext();

        vkDestroyDescriptorPool(VulkanRenderer::device(), descriptorPool, nullptr);
    }

    void VulkanImGuiLayer::startImGuiFrame() {
        framebuffer->bind();

        ImGui_ImplVulkan_NewFrame();

#ifdef TK_WIN32
        ImGui_ImplWin32_NewFrame();
#endif

#ifdef TK_GLFW
        ImGui_ImplGlfw_NewFrame();
#endif

        ImGui::NewFrame();
    }

    void VulkanImGuiLayer::endImGuiFrame() {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanRenderer::commandBuffer());

        framebuffer->unbind();
    }

}