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

        ImGui_ImplGlfw_InitForVulkan(Application::getWindow()->getHandle(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = VulkanRenderer::getInstance();
        init_info.PhysicalDevice = VulkanRenderer::getPhysicalDevice();
        init_info.Device = VulkanRenderer::getDevice();
        init_info.QueueFamily = VulkanDevice::getQueueFamilyIndexes().graphicsQueueIndex;
        init_info.Queue = VulkanRenderer::getGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = VulkanDevice::getImageCount();
        init_info.ImageCount = VulkanRenderer::MAX_FRAMES;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info, VulkanRenderer::getRenderPass());

        {
            VkCommandBuffer commandBuffer = VulkanRenderer::startCommandBuffer();
            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            VulkanRenderer::endCommandBuffer(commandBuffer);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void ImGuiLayer::onDetach() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(VulkanRenderer::getDevice(), descriptorPool, nullptr);
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
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanRenderer::getCommandBuffer());
    }

}