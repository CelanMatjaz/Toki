#include "tkpch.h"
#include "prog_layer.h"

ProgLayer::ProgLayer() {
    using namespace Toki;

    Toki::VulkanPipeline::DestriptorPoolSizes sizes{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Application::getVulkanRenderer()->MAX_FRAMES }, // 1 per frame
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 }, // lights
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
    };

    auto descriptorPool = Toki::VulkanPipeline::createDescriptorPool(sizes);

    auto descriptorSetLayoutLights = Toki::VulkanPipeline::createDescriptorSetLayout({
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT },
        // { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_LIGHTS, VK_SHADER_STAGE_VERTEX_BIT }
    });

    auto descriptorSetLayoutTextures = Toki::VulkanPipeline::createDescriptorSetLayout({
        { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32, VK_SHADER_STAGE_FRAGMENT_BIT }
    });

    descriptorSetLights = Toki::VulkanPipeline::createDescriptorSets(descriptorPool, { descriptorSetLayoutLights })[0];
    descriptorSetTextures = Toki::VulkanPipeline::createDescriptorSets(descriptorPool, { descriptorSetLayoutTextures })[0];

    pipelineLayout = Toki::VulkanPipeline::createPipelineLayout({ descriptorSetLayoutLights, descriptorSetLayoutTextures }, { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant) } });

    uint32_t data[] = { 0xffff00ff, 0xff000000, 0xff000000, 0xffff00ff };
    noTexture = Toki::VulkanTexture::create(2, 2, data);
    testTexture1 = Toki::VulkanTexture::create("./assets/textures/spongebob.png");
    testTexture2 = Toki::VulkanTexture::create("./assets/textures/sphere_emoji.png");

    sampler = Toki::VulkanTexture::createSampler();

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
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Toki::Vertex, normal) },
        { 3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, position) },
        { 4, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, scale) },
        { 5, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, rotation) },
        { 6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, color) },
    };
    config.wireframe = false;

    pipeline = Toki::VulkanPipeline::createPipeline(config);

    loadedModel[0].loadModelFromObj("assets/models/spongebob_high_poly_tris.obj");
    loadedModel[1].loadModelFromObj("assets/models/sphere.obj");
    resetInstances(1, 1, 0, 0, randomParams);
    Toki::VulkanBufferData instanceData{ sizeof(Toki::InstanceData), instances };
    loadedModel[selectedModel].setInstances(&instanceData);

    camera.moveForward(-20);

    vkGetPhysicalDeviceProperties(Application::getVulkanRenderer()->getPhysicalDevice(), &properties);

    lightUniform = Toki::VulkanBuffer::create(&lightUniformSpec);

    if (1) {
        LightUniform t = { {}, 1, .1f };
        t.lights[0] = { { 10.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        light = static_cast<LightUniform*>(lightUniform->mapMemory());
        memcpy(light, &t, sizeof(LightUniform));

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = lightUniform->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(LightUniform);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext = nullptr;
        descriptorWrites[0].dstSet = descriptorSetLights;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(Application::getVulkanRenderer()->getDevice(), 1, descriptorWrites.data(), 0, nullptr);
    }

    if (1) {
        std::vector<VkDescriptorImageInfo> descriptorImageInfos(32);
        for (uint32_t i = 0; i < 32; ++i) {
            descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfos[i].imageView = noTexture->getImageView();
            descriptorImageInfos[i].sampler = sampler;
        }

        descriptorImageInfos[0].imageView = testTexture1->getImageView();
        descriptorImageInfos[1].imageView = testTexture2->getImageView();

        VkWriteDescriptorSet descriptorWrites{};
        descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites.dstSet = descriptorSetTextures;
        descriptorWrites.dstBinding = 0;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites.descriptorCount = descriptorImageInfos.size();
        descriptorWrites.pImageInfo = descriptorImageInfos.data();

        vkUpdateDescriptorSets(Application::getVulkanRenderer()->getDevice(), 1, &descriptorWrites, 0, nullptr);
    }
}

ProgLayer::~ProgLayer() {
    using namespace Toki;

    VkDevice device = Application::getVulkanRenderer()->getDevice();

    vkDeviceWaitIdle(device);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

void ProgLayer::onImGuiUpdate(float deltaTime) {
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

    ImGui::SeparatorText("Statistics");

    ImGui::Text("Device:  %s", properties.deviceName);
    // ImGui::Text("Version: %i", properties.driverVersion);
    ImGui::Text("Type:    %s", mapDeviceType(properties.deviceType));
    ImGui::NewLine();

    ImGui::Text("%.5f ms/frame", prevDelta * 1000);
    ImGui::Text("%i FPS", fps);

    static const char* presentModeStrings[] = { "Capped", "Uncapped" };

    static VkPresentModeKHR stuff[] = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR };
    static int selectedItem = 0;
    if (ImGui::Combo("Frame cap", &selectedItem, presentModeStrings, 2)) {
        Toki::Application::getVulkanRenderer()->recreateSwapchain(stuff[selectedItem]);
    }
    ImGui::NewLine();

    ImGui::SeparatorText("Lighting");
    ImGui::SliderFloat3("Light position", (float*) &light->lights[0].position, -20.0f, 20.0f);
    ImGui::SliderFloat("Ambient light amount", &light->ambientLight, 0.0f, 0.5f);
    ImGui::ColorEdit4("Light color", (float*) &light->lights[0].color);
    ImGui::NewLine();

    auto updateInstances = [this]() {
        resetInstances(nInstancesX, nInstancesY, instanceDistances[0], instanceDistances[1], randomParams);
        Toki::VulkanBufferData instanceData{ nInstancesX * nInstancesY * sizeof(Toki::InstanceData), instances };
        loadedModel[selectedModel].setInstances(&instanceData);
    };

    ImGui::SeparatorText("Instances");
    if (ImGui::SliderInt("Instance count X direction", &nInstancesX, 1, 32))
        updateInstances();

    if (ImGui::SliderInt("Instance count Z direction", &nInstancesY, 1, 32))
        updateInstances();

    if (ImGui::SliderFloat2("Instance distances", instanceDistances, 1.0f, 10.0f))
        updateInstances();

    if (ImGui::Checkbox("Random parameters", &randomParams))
        updateInstances();

    if (ImGui::Combo("Model", &selectedModel, "Spongebob\0Satellite\0"))
        updateInstances();

    if (ImGui::Button("Reload instances")) {
        updateInstances();
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

void ProgLayer::onUpdate(float deltaTime) {
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

    std::vector<VkDescriptorSet> sets = { descriptorSetLights, descriptorSetTextures };
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    PushConstant push{ camera.getProjection() * camera.getView() * glm::mat4(1.0f) };
    push.textureIndex = selectedModel;
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);

    VkViewport pipelineViewport{};
    pipelineViewport.x = 0.0f;
    pipelineViewport.y = 0.0f;
    pipelineViewport.width = (float) dimensions.width;
    pipelineViewport.height = (float) dimensions.height;
    pipelineViewport.minDepth = 0.0f;
    pipelineViewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &pipelineViewport);

    VkRect2D pipelineScissor{};
    pipelineScissor.extent = Toki::Application::getVulkanRenderer()->getSwapchain()->getExtent();
    vkCmdSetScissor(cmd, 0, 1, &pipelineScissor);

    loadedModel[selectedModel].draw(cmd);
}

void ProgLayer::onEvent(Toki::Event& e) {
    camera.onEvent(e);
}

void ProgLayer::resetInstances(uint32_t nX, uint32_t nY, float distanceX, float distanceY, bool randomParams) {
    if (instances) delete[] instances;
    instances = new Toki::InstanceData[nX * nY]();

    float halfTotalDistanceX = nX > 1 ? (nX - 1) * distanceX / 2 : 0;
    float halfTotalDistanceY = nY > 1 ? (nY - 1) * distanceY / 2 : 0;

    for (uint32_t y = 0; y < nY; ++y) {
        for (uint32_t x = 0; x < nX; ++x) {
            instances[x + y * nX].position = { x * distanceX - halfTotalDistanceX, 0.0f, y * distanceY - halfTotalDistanceY };

            Toki::InstanceData* temp = &instances[x + y * nX];

            if (!randomParams) continue;

            // instances[x + y * nX].rotation = { glm::radians(rand() % 180 - 90.0f), glm::radians(rand() % 180 / 1.0f), glm::radians(rand() % 180 - 90.0f) };
            instances[x + y * nX].rotation = { 0.0f, glm::radians(rand() % 180 / 1.0f), 0.0f };
            instances[x + y * nX].scale = { rand() % 200 / 100.0f, rand() % 200 / 100.0f, rand() % 200 / 100.0f };
            instances[x + y * nX].color = { rand() % 256 / 256.0f, rand() % 256 / 256.0f, rand() % 256 / 256.0f, 1.0f };
        }
    }
}