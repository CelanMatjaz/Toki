#include "tkpch.h"

#include "toki/core/toki.h"


class TestApplication : public Toki::Application {
public:
    TestApplication() : Toki::Application() {
        float vertexData[] = {
            -0.7f, -0.7f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -0.7f, 0.7f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            0.7f, 0.7f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            0.7f, -0.7f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        };
        vertexBuffer = Toki::VulkanBuffer::createVertexBuffer(sizeof(vertexData), vertexData);

        uint32_t indexData[] = { 0, 1, 2, 2, 3, 0 };
        indexBuffer = Toki::VulkanBuffer::createIndexBuffer(sizeof(indexData), indexData);

        pipelineLayout = Toki::VulkanPipeline::createPipelineLayout({}, {});

        Toki::VulkanPipeline::PipelineConfig config;
        config.pipelineLayout = pipelineLayout;
        config.inputBindingDescriptions = { { 0, sizeof(float) * 7, vk::VertexInputRate::eVertex } };
        config.inputAttributeDescriptions = { { 0, 0, vk::Format::eR32G32B32Sfloat, 0 }, { 1, 0, vk::Format::eR32G32B32A32Sfloat, sizeof(float) * 3 } };
        shader = Toki::VulkanShader::create("./shaders/compiled/shader.vert.spv", "./shaders/compiled/shader.frag.spv", config);

    }
    ~TestApplication() {
        vk::Device device = Toki::VulkanRenderer::getDevice();
        device.waitIdle();

        indexBuffer->cleanup();
        vertexBuffer->cleanup();
        shader->cleanup();
        device.destroyPipelineLayout(pipelineLayout);
    };

private:
    void onUpdate(float deltaTime) override {
        using namespace Toki;

        shader->recreatePipeline();

        vk::CommandBuffer cmd = VulkanRenderer::getCommandBuffer();

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, shader->getPipeline());

        cmd.bindVertexBuffers(0, { vertexBuffer->getBuffer() }, { 0 }, {});
        cmd.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);

        cmd.drawIndexed(6, 1, 0, 0, 0);
    }

    std::unique_ptr<Toki::VulkanBuffer> vertexBuffer;
    std::unique_ptr<Toki::VulkanBuffer> indexBuffer;
    std::unique_ptr<Toki::VulkanShader> shader;
    vk::PipelineLayout pipelineLayout;
};

int main(int argc, const char** argv) {
    glfwInit();

    TestApplication app;
    app.run();

    glfwTerminate();
    return 0;
}