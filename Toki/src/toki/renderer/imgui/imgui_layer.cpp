#include "imgui_layer.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "toki/core/application.h"
#include "toki/renderer/vulkan/vulkan_pipeline.h"

namespace Toki {

    void ImGuiLayer::onEvent(Event& e) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) e.setHandled();
    }

    void ImGuiLayer::onAttach() {
        descriptorPool = VulkanPipeline::createDescriptorPool({
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
        });

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();

        VulkanRenderer* renderer = Application::getVulkanRenderer();

        ImGui_ImplGlfw_InitForVulkan(Application::getWindow()->getHandle(), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = renderer->getInstance();
        initInfo.PhysicalDevice = renderer->getPhysicalDevice();
        initInfo.Device = renderer->getDevice();
        initInfo.QueueFamily = renderer->getQueueFamilyIndexes().graphicsQueueIndex;
        initInfo.Queue = renderer->getGraphicsQueue();
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = descriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = VulkanDevice::getImageCount();
        initInfo.ImageCount = renderer->MAX_FRAMES;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&initInfo, renderer->getSwapchain()->getRenderPass());

        {
            VkCommandBuffer commandBuffer = renderer->startCommandBuffer();
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            renderer->endCommandBuffer(commandBuffer);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void ImGuiLayer::onDetach() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(Application::getVulkanRenderer()->getDevice(), descriptorPool, nullptr);
    }

    void ImGuiLayer::onUpdate(float deltaTime) {
    }

    void ImGuiLayer::startImGuiFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::endImGuiFrame() {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Application::getVulkanRenderer()->getCommandBuffer());
    }

}