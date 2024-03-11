#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <print>

#include "toki.h"

class TestLayer : public Toki::Layer {
public:
    TestLayer() : Toki::Layer(){};
    ~TestLayer() = default;

    void onAttach() override {
        std::vector<Toki::Attachment> attachments;

        Toki::Attachment presentAttachment{};
        presentAttachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_RGBA;
        presentAttachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;
        presentAttachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;
        presentAttachment.presentable = true;
        attachments.emplace_back(presentAttachment);

        Toki::Attachment depthAttachment{};
        depthAttachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_DEPTH_STENCIL;
        depthAttachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;
        attachments.emplace_back(depthAttachment);

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
                glm::vec2 uv;
            };

            const Vertex vertices[] = {
                { { 400, -600, 2.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 1.0f } },                           // vertex 1
                { { 800, 0, -1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },                             // vertex 2
                { { 0, 0, -1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },                               // vertex 3
                { { -0.5 * 800 + 400, -0.5 * 600 - 300, 0.0f }, { 0.5f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },  // vertex 6
                { { 0.5 * 800 + 400, -0.5 * 600 - 300, 0.0f }, { 1.0f, 0.5f, 1.0f, 1.0f }, { 0.0f, 0.0f } },   // vertex 5
                { { 0.0 * 800 + 400, 0.5 * 600 - 300, 0.0f }, { 1.0f, 1.0f, 0.5f, 1.0f }, { 0.0f, 0.0f } },    // vertex 4
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
            config.shaderStagePaths[Toki::ShaderStage::SHADER_STAGE_FRAGMENT] = "assets/shaders/test_shader.frag.glsl";
            config.shaderStagePaths[Toki::ShaderStage::SHADER_STAGE_VERTEX] = "assets/shaders/test_shader.vert.glsl";
            config.layoutDescriptions.attributeDescriptions = {
                { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, 0 },
                { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT4, 4 * sizeof(float) },
                { 2, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, 7 * sizeof(float) },
            };
            config.layoutDescriptions.bindingDescriptions = { { 0, 9 * sizeof(float), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX } };
            config.attachments = attachments;
            shader = Toki::Shader::create(config);
        }

        camera.setProjection(glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 1000.0f));
        camera.setView(glm::lookAt(glm::vec3{ 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));

        {
            mvp = camera.getProjection() * camera.getView() * glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });

            Toki::UniformBufferConfig config{};
            config.size = sizeof(mvp);
            config.setIndex = 0;
            config.binding = 0;
            config.arrayElement = 0;
            uniformBuffer = Toki::UniformBuffer::create(config);
            mappedMemory = (glm::mat4*) uniformBuffer->mapMemory(config.size, 0);
            memcpy(mappedMemory, &mvp, sizeof(mvp));
        }

        {
            Toki::UniformBufferConfig config{};
            config.size = sizeof(glm::vec3);
            config.setIndex = 0;
            config.binding = 1;
            config.arrayElement = 0;
            offsetUniform = Toki::UniformBuffer::create(config);
            offset = (glm::vec3*) offsetUniform->mapMemory(config.size, 0);
            memset(offset, 0, sizeof(glm::vec3));
        }

        {
            Toki::SamplerConfig config{};
            config.setIndex = 1;
            config.binding = 1;
            config.arrayElement = 0;
            testSampler = Toki::Sampler::create(config);
        }

        {
            Toki::TextureConfig config{};
            config.setIndex = 1;
            config.binding = 0;
            config.arrayElement = 0;
            config.optionalSampler = testSampler;
            testTexture = Toki::Texture::create("assets/textures/chad.jpg", config);
        }

        shader->setUniforms({ uniformBuffer, offsetUniform, testTexture });

        Toki::Event::bindEvent(Toki::EventType::WindowResize, this, [this](void* sender, void* receiver, const Toki::Event& event) {
            glm::mat4& mvp = *this->mappedMemory;
            mvp = camera.getProjection() * camera.getView() * glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
        });
    }

    Toki::OrthographicCamera camera;
    glm::mat4 mvp;
    glm::mat4* mappedMemory;
    glm::vec3* offset;

    void onRender() override {
        submit(renderPass, [this](const Toki::RenderingContext& ctx) {
            // mvp = camera.getProjection() * camera.getView() * glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
            offset->r += 0.1;

            ctx.bindVertexBuffers({ vertexBuffer });
            ctx.bindIndexBuffer(indexBuffer);
            ctx.bindShader(shader);
            ctx.pushConstants(shader, sizeof(mvp), &mvp);
            ctx.bindUniforms(shader, 0, 2);
            ctx.draw(3, 1, 0, 0);
            // ctx.draw(3, 1, 3, 0);
        });
    }

private:
    Toki::Ref<Toki::RenderPass> renderPass;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::IndexBuffer> indexBuffer;
    Toki::Ref<Toki::UniformBuffer> uniformBuffer;
    Toki::Ref<Toki::UniformBuffer> offsetUniform;
    Toki::Ref<Toki::Shader> shader;
    Toki::Ref<Toki::Texture> testTexture;
    Toki::Ref<Toki::Sampler> testSampler;
};

int main() {
    Toki::ApplicationConfig applicationConfig{};
    applicationConfig.windowConfig.isResizable = true;

    Toki::Application app{ applicationConfig };

    app.pushLayer(Toki::createRef<TestLayer>());

    app.start();

    return 0;
}
