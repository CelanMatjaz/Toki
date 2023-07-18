#include "toki/core/toki.h"

#include "spirv_cross/spirv_reflect.hpp"

// temp code
Toki::Vertex vertexData[4] = {
    { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
};

uint32_t indexData[] = {
    0, 1, 2, 2, 3, 0,
    // 3, 2, 1, 3, 1, 0,
};

struct UniformData {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
};

struct PushConstant {
    glm::mat4 mvp;
};
// temp code

class TestLayer : public Toki::Layer {
public:
    TestLayer() {
        using namespace Toki;

        Toki::VulkanPipeline::DestriptorPoolSizes sizes{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32 },
        };

        auto descriptorPool = Toki::VulkanPipeline::createDescriptorPool(sizes);

        auto descriptorSetLayout = Toki::VulkanPipeline::createDescriptorSetLayout({
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT }, });

        pipelineLayout = Toki::VulkanPipeline::createPipelineLayout({ }, { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant) } });

        Toki::VulkanPipeline::PipelineConfig config{};
        config.vertShaderIndex = Toki::VulkanPipeline::createShaderModule("./shaders/compiled/basic.vert.spv");
        config.fragShaderIndex = Toki::VulkanPipeline::createShaderModule("./shaders/compiled/basic.frag.spv");
        config.pipelineLayout = pipelineLayout;
        config.inputBindingDescriptions = {
            { 0, sizeof(Toki::Vertex), VK_VERTEX_INPUT_RATE_VERTEX },
            { 1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE },
        };
        config.inputAttributeDescriptions = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Toki::Vertex, position) },
            { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Toki::Vertex, uv) },
            { 2, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, position) },
            { 3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, scale) },
            { 4, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, rotation) },
            { 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, color) },
        };
        config.wireframe = false;

        pipeline = Toki::VulkanPipeline::createPipeline(config);

        model.loadModelData(vertexData, sizeof(vertexData), indexData, sizeof(indexData));
        model.addInstance(&singleInstance);
        singleInstance.position = { -2.0f, -2.0f, 0.0f };
        singleInstance.scale.x *= 2;
        // singleInstance.rotation.x = glm::radians(45.0f);
        singleInstance.color = { 1.0, 1.0, 0, 1.0 };
        model.addInstance(&singleInstance);

        cubeModel.loadModelFromObj("assets/models/cube.obj");

        singleInstance.position = glm::vec3{ 0.0f };
        singleInstance.scale = glm::vec3{ 1.0f };
        singleInstance.color = { 1.0f, 0.0f, 1.0f, 1.0f };
        cubeModel.addInstance(&singleInstance);

        camera.moveForward(-20);

        vkGetPhysicalDeviceProperties(Application::getVulkanRenderer()->getPhysicalDevice(), &properties);
    }

    Toki::InstanceData singleInstance = { { 2.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    Toki::Model model;
    Toki::Model cubeModel;
    VkPhysicalDeviceProperties properties{};


    ~TestLayer() {
        using namespace Toki;

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        vkDeviceWaitIdle(device);

        vertexBuffer->cleanup();
        indexBuffer->cleanup();
        instanceBuffer->cleanup();

        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void onImGuiUpdate(float deltaTime) override {
        static uint32_t fps = 0;
        static uint32_t frameCount = 0;
        static uint32_t prevTime = 0;
        static float prevDelta = deltaTime;

        uint32_t currentTime = glfwGetTime();

        ImGui::Begin("Stats");

        auto mapDeviceType = [](VkPhysicalDeviceType type) {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
                case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                default:
                    return "Other";
            }
            };

        ImGui::Text("Device:  %s", properties.deviceName);
        ImGui::Text("Version: %i", properties.driverVersion);
        ImGui::Text("Type:    %s", mapDeviceType(properties.deviceType));
        ImGui::NewLine();

        ImGui::Text("%.5f ms/frame", prevDelta * 1000);
        ImGui::Text("%i FPS", fps);



        static const char* presentModeStrings[] = { "FIFO", "FIFO_RELAXED", "IMMEDIATE", "MAILBOX" };
        static VkPresentModeKHR stuff[] = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,VK_PRESENT_MODE_MAILBOX_KHR };
        static int selectedItem = 0;
        if (ImGui::Combo("Present mode", &selectedItem, presentModeStrings, 4)) {
            Toki::Application::getVulkanRenderer()->recreateSwapchain(stuff[selectedItem]);
        }

        ImGui::End();

        ++frameCount;

        if (currentTime - prevTime >= 1.0) {
            fps = frameCount;
            frameCount = 0;
            prevDelta = deltaTime;
        }

        prevTime = currentTime;
    }

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    UniformData uniformData{};

    std::shared_ptr<Toki::VulkanBuffer> uniformBuffer;
    std::shared_ptr<Toki::VulkanBuffer> vertexBuffer = Toki::VulkanBuffer::createVertexBuffer(sizeof(vertexData), &vertexData);
    std::shared_ptr<Toki::VulkanBuffer> indexBuffer = Toki::VulkanBuffer::createIndexBuffer(sizeof(indexData), &indexData);
    std::shared_ptr<Toki::VulkanBuffer> instanceBuffer = Toki::VulkanBuffer::createVertexBuffer(100 * sizeof(Toki::InstanceData), &singleInstance);

    Toki::Camera camera{ Toki::Camera::CameraProjection::PERSPECTIVE, Toki::Camera::CameraType::FIRST_PERSON, { 0.0f, 0.0f, 0.0f } };

    float speed = 10.0f;

    void onAttach() override {
        using namespace Toki;
        GLFWwindow* win = Application::getNativeWindow();
        auto [width, height] = Toki::Application::getWindow()->getWindowDimensions();
        // glfwSetCursorPos(win, width / 2, height / 2);
    }

    bool cursorInFocus = false;

    void onUpdate(float deltaTime) override {
        using namespace Toki;

        GLFWwindow* win = Application::getNativeWindow();
        double xPos, yPos;
        glfwGetCursorPos(win, &xPos, &yPos);

        auto dimensions = Toki::Application::getWindow()->getWindowDimensions();

        if (1) {
            if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
                camera.moveLeft(+speed * deltaTime);
            }
            if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
                camera.moveLeft(-speed * deltaTime);
            }
            if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                camera.moveUp(-speed * deltaTime);
            }
            if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                camera.moveUp(+speed * deltaTime);
            }
            if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
                camera.moveForward(+speed * deltaTime);
            }
            if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
                camera.moveForward(-speed * deltaTime);
            }

            if (cursorInFocus) {
                camera.rotateLeft(-(dimensions.width / 2 - xPos) * deltaTime);
                camera.rotateUp(-(dimensions.height / 2 - yPos) * deltaTime);
                glfwSetCursorPos(win, dimensions.width / 2, dimensions.height / 2);
            }

            if (!cursorInFocus && glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                cursorInFocus = true;
                glfwSetCursorPos(win, dimensions.width / 2, dimensions.height / 2);
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            if (cursorInFocus && (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)) {
                cursorInFocus = false;
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        VkCommandBuffer cmd = Toki::Application::getVulkanRenderer()->getCommandBuffer();

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        PushConstant push{ camera.getProjection() * camera.getView() * glm::mat4(1.0f) };
        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);

        VkViewport pipelineViewport{};
        pipelineViewport.x = 0.0f;
        pipelineViewport.y = 0.0f;
        pipelineViewport.width = (float)dimensions.width;
        pipelineViewport.height = (float)dimensions.height;
        pipelineViewport.minDepth = 0.0f;
        pipelineViewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &pipelineViewport);

        VkRect2D pipelineScissor{};
        pipelineScissor.extent = Toki::Application::getVulkanRenderer()->getSwapchain()->getExtent();
        vkCmdSetScissor(cmd, 0, 1, &pipelineScissor);

        model.draw(cmd);
        cubeModel.draw(cmd);
    }

};

int main(int argc, const char** argv) {
    Toki::Application app(Toki::RendererAPI::VULKAN);
    app.addLayer(new TestLayer());
    app.run();

    return 0;
}