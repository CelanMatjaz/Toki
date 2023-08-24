#include "iostream"
#include "filesystem"
#include "toki.h"

class TempLayer: public Toki::Layer {
public:
    TempLayer() {}

    void onAttach() override {
        using namespace Toki;

        FramebufferConfig framebufferConfig{};
        framebufferConfig.target = RenderTarget::Swapchain;
        framebufferConfig.width = 1280;
        framebufferConfig.height = 720;
        framebufferConfig.clearColor = { 0.01f, 0.01f, 0.01f, 1.0f };
        framebufferConfig.depthAttachment = createRef<Attachment>(Format::Depth, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::DontCare);
        framebufferConfig.colorAttachments = {
            { Format::RGBA8, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, RenderTarget::Swapchain }
        };
        framebuffer = Framebuffer::create(framebufferConfig);

        Toki::ShaderConfig shaderConfig{};
        shaderConfig.framebuffer = framebuffer;
        shaderConfig.path = "assets/shaders/raw/test.shader.glsl";
        shaderConfig.type = Toki::ShaderType::Graphics;
        // locaiton, binding, format, offset
        shaderConfig.attributeDescriptions = {
            { 0, 0, VertexFormat::Float2, 0 },
            { 1, 1, VertexFormat::Float2, 0 }
        };
        // binding, stride, inputRate
        shaderConfig.bindingDescriptions = {
            { 0, 2 * sizeof(float), VertexInputRate::Vertex },
            { 1, 2 * sizeof(float), VertexInputRate::Instance }
        };
        shader = Toki::Shader::create(shaderConfig);

        {
            glm::vec2 positions[3] = {
                { 0.0f, -0.2f },
                { -0.2f, 0.2f },
                { 0.2f, 0.2f },
            };

            VertexBufferConfig vertexBufferConfig{};
            vertexBufferConfig.size = sizeof(positions);
            vertexBuffer = VertexBuffer::create(vertexBufferConfig);
            vertexBuffer->setData(sizeof(positions), positions);
        }

        {
            glm::vec2 instancePositions[3] = {
                { 0.0f, -0.5f },
                { -0.5f, 0.5f },
                { 0.5f, 0.5f },
            };

            VertexBufferConfig instanceBufferConfig{};
            instanceBufferConfig.size = sizeof(instancePositions);
            instanceBufferConfig.binding = 1;
            instanceBuffer = VertexBuffer::create(instanceBufferConfig);
            instanceBuffer->setData(sizeof(instancePositions), instancePositions);
        }

        {
            uint32_t indicies[] = { 0, 1, 2 };

            IndexBufferConfig indexBufferConfig{};
            indexBufferConfig.size = 3 * sizeof(uint32_t);
            indexBufferConfig.indexCount = 3;
            indexBuffer = IndexBuffer::create(indexBufferConfig);
            indexBuffer->setData(sizeof(indicies), indicies);
        }

        {
            glm::vec4 addedColor = { .2, .2, .2, 1 };

            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(glm::vec4);
            uniformBuffer = UniformBuffer::create(uniformBufferConfig);
            uniformBuffer->setData(sizeof(addedColor), &addedColor);
        }


    }

    void onUpdate(float deltaTime) override {
        framebuffer->bind();
        shader->bind();

        Toki::RendererCommand::setViewport({ 0, 0 }, { 1280, 720 });
        Toki::RendererCommand::setScissor({ 0, 0 }, { 1280, 720 });

        glm::vec2 offsetPositions{ .0, .0 };

        Toki::RendererCommand::setConstant(shader, Toki::ShaderStage::Vertex, sizeof(glm::vec2), &offsetPositions);

        static float added1 = 0;
        static float added2 = 0;
        static float added3 = 0;
        added1 += ((.00001));
        added2 += ((.0002));
        added3 += ((.003));

        if (added1 > 1) added1 -= 1;
        if (added2 > 1) added2 -= 1;
        if (added3 > 1) added3 -= 1;



        glm::vec4 addedColor = { added1, added2, added3, 1 };
        uniformBuffer->setData(sizeof(addedColor), &addedColor);

        Toki::RendererCommand::setUniform(shader, uniformBuffer, Toki::ShaderStage::Fragment, 0, 0);

        Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer, 3);

        framebuffer->unbind();
    }

private:
    Toki::Ref<Toki::Shader> shader;
    Toki::Ref<Toki::Framebuffer> framebuffer;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::VertexBuffer> instanceBuffer;
    Toki::Ref<Toki::UniformBuffer> uniformBuffer;
    Toki::Ref<Toki::IndexBuffer> indexBuffer;
};

int main(int argc, char** argv) {

    Toki::EngineConfig config{};
    config.workingDirectory = std::filesystem::absolute(argc == 1 ? std::filesystem::path(".") : argv[1]);

    Toki::Engine app(config);

    app.pushLayer(Toki::createRef<TempLayer>());

    app.run();

    return 0;
}