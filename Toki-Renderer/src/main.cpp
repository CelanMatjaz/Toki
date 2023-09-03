#include "iostream"
#include "filesystem"
#include "toki.h"
#include "imgui.h"

class TempLayer : public Toki::Layer {
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
            { 0, 0, VertexFormat::Float3, offsetof(Toki::Vertex, position) },
            { 1, 0, VertexFormat::Float2, offsetof(Toki::Vertex, uv)},
            { 2, 0, VertexFormat::Float3, offsetof(Toki::Vertex, normal) },
            { 3, 1, VertexFormat::Float3, 0 }
        };
        // binding, stride, inputRate
        shaderConfig.bindingDescriptions = {
            { 0, sizeof(Toki::Vertex), VertexInputRate::Vertex },
            { 1, sizeof(glm::vec3), VertexInputRate::Instance }
        };
        shader = Toki::Shader::create(shaderConfig);

        {
            Vertex positions[] = {
                { { -10.0f, -1.0f, -10.0f }, {1.0f, 1.0f }, {} },
                { { 10.0f, -1.0f, -10.0f }, {1.0f, 0.0f}, {} },
                { { -10.0f, 0.0f, 10.0f }, {0.0f, 1.0f}, {} },
                { { 10.0f, 0.0f, 10.0f }, {0.0f, 0.0f}, {} },
            };

            VertexBufferConfig vertexBufferConfig{};
            vertexBufferConfig.size = sizeof(positions);
            vertexBufferConfig.binding = 0;
            vertexBuffer = VertexBuffer::create(vertexBufferConfig);
            vertexBuffer->setData(sizeof(positions), positions);
        }

        {
            glm::vec3 instancePositions[] = {
                { 0.0f, -0.5f, 0.0f },
                // { -0.5f, 0.5f, 0.0f },
                // { 0.5f, 0.5f, 0.0f },
            };

            VertexBufferConfig instanceBufferConfig{};
            instanceBufferConfig.size = sizeof(instancePositions);
            instanceBufferConfig.binding = 1;
            instanceBuffer = VertexBuffer::create(instanceBufferConfig);
            instanceBuffer->setData(sizeof(instancePositions), instancePositions);
        }

        {
            uint32_t indicies[] = { 2, 1, 0, 1, 2, 3 };

            IndexBufferConfig indexBufferConfig{};
            indexBufferConfig.size = sizeof(indicies);
            indexBufferConfig.indexCount = sizeof(indicies) / sizeof(uint32_t);
            indexBuffer = IndexBuffer::create(indexBufferConfig);
            indexBuffer->setData(sizeof(indicies), indicies);
        }

        {
            struct {
                glm::vec4 addedColor = { .2, .2, .2, 1 };
                uint32_t textureIndex = 0;
            } color;

            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(color);
            uniformBuffer = UniformBuffer::create(uniformBufferConfig);
            uniformBuffer->setData(sizeof(color), &color);
        }

        {
            Toki::TextureConfig textureConfig{};
            textureConfig.path = "assets/textures/spongebob.png";
            texture = Toki::Texture::create(textureConfig);
        }

        model = Geometry::create();
        model->loadFromObj("assets/models/spongebob_high_poly_tris.obj");

        camera = Toki::createRef<CameraController>(glm::radians(60.0f), 1280 / 720.f, 0.1f, 1000.0f);

    }

    struct Push {
        glm::mat4 mvp;
        glm::vec2 offsetPositions{ .0, .0 };
    };

    void onUpdate(float deltaTime) override {
        framebuffer->bind();
        shader->bind();

        Toki::RendererCommand::setViewport({ 0, 0 }, { 1280, 720 });
        Toki::RendererCommand::setScissor({ 0, 0 }, { 1280, 720 });

        static float rotation = 0;

        camera->onUpdate();



        Push push;
        push.mvp = camera->getProjection() * camera->getView() *
            // push.mvp = camera->getProjection() *
            glm::scale(glm::translate(glm::mat4{ 1.0f }, { 0.0f, 0.0f, -5.0f }), { .4f, .4f, .4f });

        Toki::RendererCommand::setConstant(shader, sizeof(Push), &push);

        static float added1 = 0;
        static float added2 = 0;
        static float added3 = 0;
        added1 += ((.00001));
        added2 += ((.0002));
        added3 += ((.003));

        if (added1 > 1) added1 -= 1;
        if (added2 > 1) added2 -= 1;
        if (added3 > 1) added3 -= 1;

        Toki::RendererCommand::setUniform(shader, uniformBuffer, 0, 0);
        Toki::RendererCommand::setTexture(shader, texture, 0, 0, 1);

        // Toki::RendererCommand::drawInstanced({ vertexBuffer ,instanceBuffer }, indexBuffer);

        Toki::RendererCommand::drawInstanced(model, instanceBuffer);

        framebuffer->unbind();
    }

    void renderImGui() override {
        ImGui::Begin("Stats");

        ImGui::DragFloat3("Front", (float*) &camera->getFront(), -1.0f, 1.0f);
        ImGui::DragFloat3("Left", (float*) &camera->getLeft(), -1.0f, 1.0f);
        ImGui::DragFloat3("Up", (float*) &camera->getUp(), -1.0f, 1.0f);

        ImGui::End();
    }

private:
    Toki::Ref<Toki::Shader> shader;
    Toki::Ref<Toki::Framebuffer> framebuffer;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::VertexBuffer> instanceBuffer;
    Toki::Ref<Toki::UniformBuffer> uniformBuffer;
    Toki::Ref<Toki::IndexBuffer> indexBuffer;

    Toki::Ref<Toki::Texture> texture;

    Toki::Ref<Toki::Geometry> model;

    Toki::Ref<Toki::CameraController> camera;
};

int main(int argc, char** argv) {

    Toki::EngineConfig config{};
    config.windowConfig.resizable = true;
    config.workingDirectory = std::filesystem::absolute(argc == 1 ? std::filesystem::path(".") : argv[1]);


    Toki::Engine app(config);

    app.pushLayer(Toki::createRef<TempLayer>());

    app.run();

    return 0;
}