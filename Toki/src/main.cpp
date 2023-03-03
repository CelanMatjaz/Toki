#include "tkpch.h"

#include "toki/core/toki.h"

class TestLayer : public Toki::Layer {
public:
    TestLayer() : Toki::Layer() {

        vertexBuffer = Toki::VulkanBuffer::createVertexBuffer(sizeof(vertexData), vertexData);

        uint32_t indexData[] = { 0, 1, 2, 2, 3, 0 };
        indexBuffer = Toki::VulkanBuffer::createIndexBuffer(sizeof(indexData), indexData);

        pipelineLayoutIndex = Toki::VulkanPipeline::createPipelineLayout({}, {});

        Toki::VulkanPipeline::PipelineConfig config;
        config.pipelineLayoutIndex = pipelineLayoutIndex;
        config.inputBindingDescriptions = { { 0, sizeof(float) * 7, VK_VERTEX_INPUT_RATE_VERTEX } };
        config.inputAttributeDescriptions = { { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 3 } };
        config.vertShaderIndex = Toki::VulkanPipeline::createShaderModule("./shaders/compiled/shader.vert.spv");
        config.fragShaderIndex = Toki::VulkanPipeline::createShaderModule("./shaders/compiled/shader.frag.spv");
        shader = Toki::VulkanShader::create(config);

        vertexBuffer->setData(sizeof(vertexData), vertexData);
    }

    ~TestLayer() {
        VkDevice device = Toki::VulkanRenderer::getDevice();
        vkDeviceWaitIdle(device);
        indexBuffer->cleanup();
        vertexBuffer->cleanup();
    };

    void onUpdate(float deltaTime) override {
        using namespace Toki;

        static float a = 0;
        a += 0.1f * deltaTime;

        if (a > 0.05f) a = 0;

        // for (uint32_t i = 0; i < 4; ++i) {
        //     vertexData[i * 7] += a;
        // }


        VkCommandBuffer cmd = VulkanRenderer::getCommandBuffer();

        shader->bind(cmd);

        VkBuffer vertexBuffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, &offsets);
        vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
    }

    float vertexData[28] = {
        -0.7f, -0.7f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.7f, 0.7f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.7f, 0.7f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.7f, -0.7f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    };

private:
    std::unique_ptr<Toki::VulkanBuffer> vertexBuffer;
    std::unique_ptr<Toki::VulkanBuffer> indexBuffer;
    std::unique_ptr<Toki::VulkanShader> shader;
    uint32_t pipelineLayoutIndex;
};

int main(int argc, const char** argv) {
    glfwInit();

    Toki::Application app;
    app.addLayer(new TestLayer());
    app.run();

    glfwTerminate();
    return 0;
}