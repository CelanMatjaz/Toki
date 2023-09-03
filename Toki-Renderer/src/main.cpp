#include "iostream"
#include "filesystem"
#include "toki.h"
#include "imgui.h"

class TempLayer : public Toki::Layer {
    struct InstanceData {
        alignas(glm::vec4) glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        alignas(glm::vec4) glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
    };

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
            { 3, 1, VertexFormat::Float3, offsetof(InstanceData, position) },
            { 4, 1, VertexFormat::Float3, offsetof(InstanceData, rotation) },
            { 5, 1, VertexFormat::Float3, offsetof(InstanceData, scale) },
            { 6, 1, VertexFormat::Float4, offsetof(InstanceData, color) },
        };


        // binding, stride, inputRate
        shaderConfig.bindingDescriptions = {
            { 0, sizeof(Toki::Vertex), VertexInputRate::Vertex },
            { 1, sizeof(InstanceData), VertexInputRate::Instance }
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
            InstanceData instances[] = {
                { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }
            };

            VertexBufferConfig instanceBufferConfig{};
            instanceBufferConfig.size = sizeof(instances);
            instanceBufferConfig.binding = 1;
            instanceBuffer = VertexBuffer::create(instanceBufferConfig);
            instanceBuffer->setData(sizeof(instances), instances);
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

            struct LightData {
                alignas(4) uint32_t lightCount = 0;
                alignas(4) float ambientLight = 0.01f;
            } lightData;

            lightData.lightCount = 1;
            lightData.ambientLight = 0.1f;

            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(LightData);
            uniformBufferLightData = UniformBuffer::create(uniformBufferConfig);
            uniformBufferLightData->setData(sizeof(LightData), &lightData);
        }

        {
            struct Light {
                glm::vec3 position;
                glm::vec4 color;
            };

            Light lights[8];

            lights[0] = { { 10.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(lights);
            uniformBufferLights = UniformBuffer::create(uniformBufferConfig);
            uniformBufferLights->setData(sizeof(lights), &lights);
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
        uint32_t textureIndex = 0;
    };

    void onUpdate(float deltaTime) override {
        framebuffer->bind();
        shader->bind();

        Toki::RendererCommand::setViewport({ 0, 0 }, { 1280, 720 });
        Toki::RendererCommand::setScissor({ 0, 0 }, { 1280, 720 });

        static float rotation = 0;

        camera->onUpdate();



        Push push{};
        push.mvp = camera->getProjection() * camera->getView() *
            // push.mvp = camera->getProjection() *
            glm::scale(glm::translate(glm::mat4{ 1.0f }, { 0.0f, 0.0f, -5.0f }), { .4f, .4f, .4f });

        Toki::RendererCommand::setConstant(shader, sizeof(Push), &push);

        Toki::RendererCommand::setUniform(shader, uniformBufferLightData, 0, 0, 0);
        Toki::RendererCommand::setUniform(shader, uniformBufferLights, 1, 0, 0);
        Toki::RendererCommand::setTexture(shader, texture, 0, 0, 1);

        Toki::RendererCommand::bindSets(shader);

        // Toki::RendererCommand::drawInstanced({ vertexBuffer ,instanceBuffer }, indexBuffer);

        Toki::RendererCommand::drawInstanced(model, instanceBuffer);

        framebuffer->unbind();
    }

    void renderImGui() override {
        static uint32_t fps = 0;
        static uint32_t frameCount = 0;
        static auto prevTime = std::chrono::high_resolution_clock::now();

        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - prevTime).count();
        prevTime = now;

        static float accDelta = deltaTime;
        accDelta += deltaTime;
        ++frameCount;

        ImGui::Begin("Stats");

        ImGui::Text("%.5f ms/frame", deltaTime * 1000);
        ImGui::Text("%i FPS", fps);

        if (accDelta >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            accDelta = 0;
        }
        ImGui::End();
    }

    InstanceData* instances = nullptr;

    void resetInstances(uint32_t nX, uint32_t nY, float distanceX, float distanceY, bool randomParams) {
        if (instances) delete[] instances;
        instances = new InstanceData[nX * nY]();

        float halfTotalDistanceX = nX > 1 ? (nX - 1) * distanceX / 2 : 0;
        float halfTotalDistanceY = nY > 1 ? (nY - 1) * distanceY / 2 : 0;

        for (uint32_t y = 0; y < nY; ++y) {
            for (uint32_t x = 0; x < nX; ++x) {
                instances[x + y * nX].position = { x * distanceX - halfTotalDistanceX, 0.0f, y * distanceY - halfTotalDistanceY };

                InstanceData* temp = &instances[x + y * nX];

                if (!randomParams) continue;

                // instances[x + y * nX].rotation = { glm::radians(rand() % 180 - 90.0f), glm::radians(rand() % 180 / 1.0f), glm::radians(rand() % 180 - 90.0f) };
                instances[x + y * nX].rotation = { 0.0f, glm::radians(rand() % 180 / 1.0f), 0.0f };
                instances[x + y * nX].scale = { rand() % 200 / 100.0f, rand() % 200 / 100.0f, rand() % 200 / 100.0f };
                instances[x + y * nX].color = { rand() % 256 / 256.0f, rand() % 256 / 256.0f, rand() % 256 / 256.0f, 1.0f };
            }
        }
    }

private:
    Toki::Ref<Toki::Shader> shader;
    Toki::Ref<Toki::Framebuffer> framebuffer;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::VertexBuffer> instanceBuffer;
    Toki::Ref<Toki::UniformBuffer> uniformBufferLightData;
    Toki::Ref<Toki::UniformBuffer> uniformBufferLights;
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