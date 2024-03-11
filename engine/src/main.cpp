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
            renderPass2 = Toki::RenderPass::create(config);

            config.attachments[0].presentable = false;
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
            struct Vertex {
                glm::vec3 position;
                glm::vec2 uv;
            };

            const Vertex vertices[] = {
                { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
                { { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
                { { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
                { { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
            };

            Toki::VertexBufferConfig config{};
            config.binding = 0;
            config.size = sizeof(vertices);
            vertexBuffer2 = Toki::VertexBuffer::create(config);
            vertexBuffer2->setData(config.size, (void*) vertices);
        }

        {
            const uint32_t indices[] = { 0, 1, 2 };

            Toki::IndexBufferConfig config{};
            config.size = sizeof(indices);
            config.indexCount = sizeof(indices) / sizeof(uint32_t);
            config.indexSize = Toki::IndexSize::INDEX_SIZE_32;
            indexBuffer = Toki::IndexBuffer::create(config);
            indexBuffer->setData(config.size, (void*) indices);
        }

        {
            const uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };

            Toki::IndexBufferConfig config{};
            config.size = sizeof(indices);
            config.indexCount = sizeof(indices) / sizeof(uint32_t);
            config.indexSize = Toki::IndexSize::INDEX_SIZE_32;
            indexBuffer2 = Toki::IndexBuffer::create(config);
            indexBuffer2->setData(config.size, (void*) indices);
        }

        {
            Toki::ShaderConfig config{};
            config.shaderStages[Toki::ShaderStage::SHADER_STAGE_FRAGMENT] = (std::filesystem::path) "assets/shaders/test_shader.frag.glsl";
            config.shaderStages[Toki::ShaderStage::SHADER_STAGE_VERTEX] = (std::filesystem::path) "assets/shaders/test_shader.vert.glsl";
            config.layoutDescriptions.attributeDescriptions = {
                { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, 0 },
                { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT4, 4 * sizeof(float) },
                { 2, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, 7 * sizeof(float) },
            };
            config.layoutDescriptions.bindingDescriptions = { { 0, 9 * sizeof(float), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX } };
            config.attachments = attachments;
            config.attachments[0].presentable = false;
            shader = Toki::Shader::create(config);

            config.shaderStages[Toki::ShaderStage::SHADER_STAGE_FRAGMENT] = (std::filesystem::path) "assets/shaders/test_shader_post.frag.glsl";
            config.shaderStages[Toki::ShaderStage::SHADER_STAGE_VERTEX] = (std::filesystem::path) "assets/shaders/test_shader_post.vert.glsl";
            config.layoutDescriptions.attributeDescriptions = {
                { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, 0 },
                { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, 3 * sizeof(float) },
            };
            config.layoutDescriptions.bindingDescriptions = { { 0, 5 * sizeof(float), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX } };
            config.attachments[0].presentable = true;
            shader2 = Toki::Shader::create(config);
        }

        camera.setProjection(glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 1000.0f));
        camera.setView(glm::lookAt(glm::vec3{ 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }));

        {
            mvp = camera.getProjection() * camera.getView() * glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });

            Toki::UniformBufferConfig config{};
            config.size = sizeof(mvp);
            uniformBuffer = Toki::UniformBuffer::create(config);
            mappedMemory = (glm::mat4*) uniformBuffer->mapMemory(config.size, 0);
            memcpy(mappedMemory, &mvp, sizeof(mvp));
        }

        {
            Toki::UniformBufferConfig config{};
            config.size = sizeof(glm::vec3);
            offsetUniform = Toki::UniformBuffer::create(config);
            offset = (glm::vec3*) offsetUniform->mapMemory(config.size, 0);
            memset(offset, 0, sizeof(glm::vec3));
        }

        {
            Toki::UniformBufferConfig config{};
            config.size = sizeof(float);
            postProcessing = Toki::UniformBuffer::create(config);
            float brightness = 0.2;
            postProcessing->setData(sizeof(float), &brightness);
        }

        {
            Toki::TextureConfig config{};
            testTexture = Toki::Texture::create("assets/textures/chad.jpg", config);
        }

        {
            Toki::SamplerConfig config{};
            testSampler = Toki::Sampler::create(config);
        }

        {
            Toki::TextureConfig config{};
            config.optionalSampler = testSampler;
            testTexture = Toki::Texture::create("assets/textures/chad.jpg", config);
        }

        shader->setUniforms({ { uniformBuffer, 0, 0 }, { offsetUniform, 0, 1 }, { testTexture, 1, 0 } });
        shader2->setUniforms({ { postProcessing, 0, 0 }, { testSampler, 1, 1 } });

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
            offset->r += 0.1;
            
            ctx.bindVertexBuffers({ vertexBuffer });
            ctx.bindIndexBuffer(indexBuffer);
            ctx.bindShader(shader);
            ctx.pushConstants(shader, sizeof(mvp), &mvp);
            ctx.bindUniforms(shader, 0, 2);
            ctx.draw(3, 1, 0, 0);
            ctx.draw(3, 1, 3, 0);
        });

        submit(renderPass2, [this](const Toki::RenderingContext& ctx) {
            shader2->setUniforms({ { renderPass->getColorAttachment(0), 1, 0 } });

            ctx.bindVertexBuffers({ vertexBuffer2 });
            ctx.bindIndexBuffer(indexBuffer2);
            ctx.bindShader(shader2);
            ctx.bindUniforms(shader2, 0, 2);
            ctx.drawIndexed(6, 1, 0, 0, 0);
        });

        static float test = 0.0;
        test += 0.01;

        Toki::Renderer2D::begin();
        Toki::Renderer2D::drawQuad(glm::vec2{ test * 4, test * test }, glm::vec2{ 300.0f, 300.0f }, glm::vec4{ 1.0f });
        Toki::Renderer2D::end();
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

    Toki::Ref<Toki::VertexBuffer> vertexBuffer2;
    Toki::Ref<Toki::IndexBuffer> indexBuffer2;
    Toki::Ref<Toki::UniformBuffer> postProcessing;
    Toki::Ref<Toki::RenderPass> renderPass2;
    Toki::Ref<Toki::Shader> shader2;
};

int main() {
    Toki::ApplicationConfig applicationConfig{};
    applicationConfig.windowConfig.isResizable = true;

    Toki::Application app{ applicationConfig };

    app.pushLayer(Toki::createRef<TestLayer>());

    app.start();

    return 0;
}
