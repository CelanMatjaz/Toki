#include <glm/glm.hpp>
#include <print>

#include "toki.h"

class TestLayer : public Toki::Layer {
public:
    TestLayer() : Toki::Layer(){};
    ~TestLayer() = default;

    void onAttach() override {
        std::vector<Toki::Attachment> attachments;
        attachments.resize(1);
        Toki::Attachment& attachment = attachments[0];
        attachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_RGBA;
        attachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;
        attachment.presentable = true;

        {
            Toki::RenderPassConfig config{};
            config.width = 800;
            config.height = 600;
            config.attachments = attachments;
            renderPass = Toki::RenderPass::create(config);
        }

        {
            struct Vertex {
                glm::vec3 position;
                glm::vec4 color;
            };

            const Vertex vertices[] = {
                { { 0.0, -0.5, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // vertex 1
                { { 0.5, 0.5, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },   // vertex 2
                { { -0.5, 0.5, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },  // vertex 3
            };

            Toki::VertexBufferConfig config{};
            config.binding = 0;
            config.size = sizeof(vertices);
            vertexBuffer = Toki::VertexBuffer::create(config);
            vertexBuffer->setData(config.size, (void*) vertices);
        }

        {
            const uint32_t indices[] = { 0, 1, 2 };

            Toki::IndexBufferConfig config{};
            config.size = sizeof(indices);
            config.indexCount = sizeof(indices) / sizeof(uint32_t);
            indexBuffer = Toki::IndexBuffer::create(config);
            indexBuffer->setData(config.size, (void*) indices);
        }

        {
            Toki::ShaderConfig config{};
            config.shaderStagePaths[Toki::ShaderStage::SHADER_STAGE_FRAGMENT] = "assets/shaders/test_shader.frag";
            config.shaderStagePaths[Toki::ShaderStage::SHADER_STAGE_VERTEX] = "assets/shaders/test_shader.vert";
            config.layoutDescriptions.attributeDescriptions = { { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, 0 },
                                                                { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT4, 3 * sizeof(float) } };
            config.layoutDescriptions.bindingDescriptions = { { 0, 7 * sizeof(float), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX } };
            shader = Toki::Shader::create(config);
        }
    }

    void onRender() override {
        submit(renderPass, [this](const Toki::RenderingContext& ctx) {
            ctx.bindVertexBuffers({ vertexBuffer });
            ctx.bindIndexBuffer(indexBuffer);
            ctx.bindShader(shader);
            ctx.draw(3, 1, 0, 0);
        });
    }

private:
    Toki::Ref<Toki::RenderPass> renderPass;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::IndexBuffer> indexBuffer;
    Toki::Ref<Toki::Shader> shader;
};

int main() {
    Toki::ApplicationConfig applicationConfig{};
    applicationConfig.windowConfig.isResizable = true;

    Toki::Application app{ applicationConfig };

    app.pushLayer(Toki::createRef<TestLayer>());

    app.start();

    return 0;
}
