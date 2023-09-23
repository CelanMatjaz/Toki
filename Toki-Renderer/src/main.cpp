#include "iostream"
#include "filesystem"
#include "toki.h"
#include "imgui.h"
#include "bitset"

class TempLayer : public Toki::Layer {
    struct InstanceData {
        alignas(glm::vec4) glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        alignas(glm::vec4) glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
        alignas(glm::vec4) float id{ 0 };
    };

    struct Light {
        glm::vec3 position;
        glm::vec4 color;
    };

    Light lights[8];

public:
    TempLayer() {}

    void onAttach() override {
        using namespace Toki;

        FramebufferConfig framebufferConfig{};
        framebufferConfig.target = RenderTarget::Swapchain;
        framebufferConfig.width = Engine::getWindow()->getWidth();
        framebufferConfig.height = Engine::getWindow()->getHeight();
        framebufferConfig.clearColor = { 0.01f, 0.01f, 0.01f, 1.0f };
        framebufferConfig.depthAttachment = createRef<Attachment>(Format::Depth, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::DontCare);
        framebufferConfig.colorAttachments = {
            { Format::RGBA8, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, RenderTarget::Texture },
            { Format::R32, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, RenderTarget::Texture }
        };
        framebuffer = Framebuffer::create(framebufferConfig);

        framebufferConfig.depthAttachment.reset();
        framebufferConfig.colorAttachments = {
            { Format::R32, Samples::Sample1, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, RenderTarget::Swapchain },
        };
        testFramebuffer = Framebuffer::create(framebufferConfig);

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
            { 7, 1, VertexFormat::Float1, offsetof(InstanceData, id) },
        };

        // binding, stride, inputRate
        shaderConfig.bindingDescriptions = {
            { 0, sizeof(Toki::Vertex), VertexInputRate::Vertex },
            { 1, sizeof(InstanceData), VertexInputRate::Instance }
        };
        shader = Toki::Shader::create(shaderConfig);




        Toki::ShaderConfig outlineShaderConfig = shaderConfig;
        outlineShaderConfig.properties.cullMode = Toki::CullMode::Front;
        outlineShaderConfig.properties.wireframe = true;
        outlineShaderConfig.path = "assets/shaders/raw/geometry_pick.shader.glsl";
        outlineShader = Toki::Shader::create(outlineShaderConfig);

        Toki::ShaderConfig singleQuadShaderConfig = shaderConfig;
        singleQuadShaderConfig.attributeDescriptions = {
            { 0, 0, VertexFormat::Float3, offsetof(Toki::Vertex, position) },
            { 1, 0, VertexFormat::Float2, offsetof(Toki::Vertex, uv)},
            { 2, 0, VertexFormat::Float3, offsetof(Toki::Vertex, normal) },
        };
        singleQuadShaderConfig.bindingDescriptions = {
            { 0, sizeof(Toki::Vertex), VertexInputRate::Vertex }
        };
        singleQuadShaderConfig.framebuffer = testFramebuffer;
        singleQuadShaderConfig.properties.cullMode = Toki::CullMode::None;
        singleQuadShaderConfig.properties.wireframe = false;
        singleQuadShaderConfig.path = "assets/shaders/raw/single_quad.shader.glsl";
        singleQuadShader = Toki::Shader::create(singleQuadShaderConfig);

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
            lights[0] = { { 10.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(lights);
            uniformBufferLights = UniformBuffer::create(uniformBufferConfig);
            uniformBufferLights->setData(sizeof(lights), &lights);
        }

        {
            UniformBufferConfig uniformBufferConfig{};
            uniformBufferConfig.size = sizeof(float);
            lighthting = UniformBuffer::create(uniformBufferConfig);
            lighthting->setData(sizeof(float), &brightness);
        }

        {
            Toki::TextureConfig textureConfig{};
            textureConfig.path = "assets/textures/spongebob.png";
            texture = Toki::Texture::create(textureConfig);
        }

        {
            Toki::Vertex vertecies[4] = {
                { { -1,-1,0 }, { 0,0 }, { 0,0,-1 } },
                { { 1,-1,0 }, { 1,0 }, { 0,0,-1 } },
                { { 1,1,0 }, { 1,1 }, { 0,0,-1 } },
                { { -1,1,0 }, { 0,1 }, { 0,0,-1 } },
            };

            Toki::VertexBufferConfig vertexBufferConfig{};
            vertexBufferConfig.size = sizeof(vertecies);
            vertexBufferConfig.binding = 0;
            singleQuadVertexBuffer = Toki::VertexBuffer::create(vertexBufferConfig);
            singleQuadVertexBuffer->setData(vertexBufferConfig.size, vertecies);
        }

        {
            uint32_t indecies[6] = { 0, 2, 1, 0, 3, 2 };

            Toki::IndexBufferConfig indexBufferConfig{};
            indexBufferConfig.size = sizeof(indecies);
            indexBufferConfig.indexCount = 6;
            singleQuadIndexBuffer = Toki::IndexBuffer::create(indexBufferConfig);
            singleQuadIndexBuffer->setData(indexBufferConfig.size, indecies);
        }


        model = Geometry::create();
        model->loadFromObj(modelPath);

        camera = Toki::createRef<CameraController>(glm::radians(60.0f), 1280 / 720.f, 0.1f, 1000.0f);
        camera->setPosition({ 0.0f, -2.0f, -10.0f });

        resetBuffers();
    }

    // std::filesystem::path modelPath = "assets/models/sphere.obj";
    std::filesystem::path modelPath = "assets/models/spongebob_high_poly_tris.obj";

    struct Push {
        glm::mat4 mvp;
        uint32_t textureIndex = 0;
    };

    void onUpdate(float deltaTime) override {
        if (shouldResetBuffers) {
            resetBuffers();
            shouldResetBuffers = false;
        }

        framebuffer->bind();
        shader->bind();

        Toki::RendererCommand::setViewport({ 0, 0 }, { 1280, 720 });
        Toki::RendererCommand::setScissor({ 0, 0 }, { 1280, 720 });

        static float rotation = 0;

        lights[0].position = { glm::cos(glm::radians(rotation)) * 10, 4.0f, glm::sin(glm::radians(rotation)) * 10 };
        uniformBufferLights->setData(sizeof(lights), &lights);

        rotation += 1.0f;

        camera->onUpdate(deltaTime);

        draw();

        auto mousePosition = Toki::Input::getMousePosition();
        // std::cout << mousePosition.x << ' ' << mousePosition.y << '\n';

        float pixel = framebuffer->readPixel(1, mousePosition.x, mousePosition.y, 0);
        uint16_t meshId = ((uint32_t) pixel) >> 16;
        uint16_t instanceId = (uint16_t) pixel;

        if (meshId > 0 && instanceId < 99999 && rendering == 2) {
            outlineShader->bind();

            struct OutlinePush {
                glm::mat4 mvp;
                glm::vec4 color;
            } outlinePush;

            outlinePush.mvp = camera->getProjection() * camera->getView() * glm::mat4{ 1.0f };
            outlinePush.color = outlineColor;

            Toki::RendererCommand::setConstant(outlineShader, sizeof(OutlinePush), &outlinePush);
            Toki::RendererCommand::setLineWidth(7.0f);
            Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer, 1, instanceId);

            outlinePush.color = { UINT8_MAX - outlinePush.color.r, UINT8_MAX - outlinePush.color.g, UINT8_MAX - outlinePush.color.b, 1.0 };
            Toki::RendererCommand::setConstant(outlineShader, sizeof(OutlinePush), &outlinePush);
            Toki::RendererCommand::setLineWidth(3.0f);
            Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer, 1, instanceId);
        }


        framebuffer->unbind();



        if (1) {
            auto idTexture = framebuffer->getAttachment(0);

            testFramebuffer->bind();

            singleQuadShader->bind();

            Toki::RendererCommand::setViewport({ 0, 0 }, { 1280, 720 });
            Toki::RendererCommand::setScissor({ 0, 0 }, { 1280, 720 });

            struct SingleQuadPush {
                glm::mat4 mvp;
            } singleQuadPush;

            singleQuadPush.mvp = glm::ortho(1.0f, -1.0f, -1.0f, 1.0f) * glm::lookAt(glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });

            Toki::RendererCommand::setTexture(singleQuadShader, idTexture, 0, 0, 0);
            Toki::RendererCommand::setUniform(singleQuadShader, lighthting, 1, 0, 0);
            Toki::RendererCommand::bindSets(singleQuadShader, 0);
            Toki::RendererCommand::setConstant(singleQuadShader, sizeof(singleQuadPush), &singleQuadPush);
            Toki::RendererCommand::draw(singleQuadVertexBuffer, singleQuadIndexBuffer);

            testFramebuffer->unbind();
        }

    }

    glm::vec4 outlineColor = { 0.7f, 0.4f, 0.1f, 1.0f };

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
        ImGui::Text("%i B", totalBufferMemorySize);
        ImGui::Text("%.5f kB", totalBufferMemorySize / 1024.0f);

        ImGui::SeparatorText("Instances");
        if (ImGui::SliderInt("Instance count X direction", &nInstancesX, 1, 32))
            shouldResetBuffers = true;

        if (ImGui::SliderInt("Instance count Z direction", &nInstancesY, 1, 32))
            shouldResetBuffers = true;

        if (ImGui::SliderFloat2("Instance distances", instanceDistances, 1.0f, 10.0f))
            shouldResetBuffers = true;

        if (ImGui::Combo("Rendering", &rendering, "Single call\0Batch rendering\0Instanced\0")) {
            shouldResetBuffers = true;
        }

        ImGui::ColorEdit4("Outline color", (float*) &outlineColor);

        if (ImGui::SliderFloat("Brightness", &brightness, 0.1f, 2.0f)) {
            lighthting->setData(sizeof(float), &brightness);
        }

        if (accDelta >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            accDelta = 0;
        }

        ImGui::End();
    }

    InstanceData* instances = nullptr;
    int nInstancesX = 3;
    int nInstancesY = 3;
    bool randomParams = false;
    float instanceDistances[2] = { 4.0f, 4.0f };

    bool shouldResetBuffers = false;

    int rendering = 2;
    uint32_t totalBufferMemorySize = 0;

    void draw() {
        switch (rendering) {
            case 0: { // Single call
                float halfTotalDistanceX = nInstancesX > 1 ? (nInstancesX - 1) * instanceDistances[0] / 2 : 0;
                float halfTotalDistanceY = nInstancesY > 1 ? (nInstancesY - 1) * instanceDistances[1] / 2 : 0;

                Push push{};

                Toki::RendererCommand::setUniform(shader, uniformBufferLightData, 0, 0, 0);
                Toki::RendererCommand::setUniform(shader, uniformBufferLights, 1, 0, 0);
                Toki::RendererCommand::setTexture(shader, texture, 0, 0, 1);

                Toki::RendererCommand::bindSets(shader);

                for (uint32_t y = 0; y < nInstancesY; ++y) {
                    for (uint32_t x = 0; x < nInstancesX; ++x) {
                        push.mvp = camera->getProjection() * camera->getView();
                        push.mvp *= glm::translate(glm::mat4{ 1.0f }, { x * instanceDistances[0] - halfTotalDistanceX, 0.0f, y * instanceDistances[1] - halfTotalDistanceY });

                        Toki::RendererCommand::setConstant(shader, sizeof(Push), &push);

                        Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer);
                    }
                }
                return;
            }
            case 1: { // Batch rendering
                Push push{};
                push.mvp = camera->getProjection() * camera->getView() * glm::mat4{ 1.0f };
                Toki::RendererCommand::setConstant(shader, sizeof(Push), &push);

                Toki::RendererCommand::setUniform(shader, uniformBufferLightData, 0, 0, 0);
                Toki::RendererCommand::setUniform(shader, uniformBufferLights, 1, 0, 0);
                Toki::RendererCommand::setTexture(shader, texture, 0, 0, 1);

                Toki::RendererCommand::bindSets(shader);

                Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer);

                return;
            }
            case 2: { // Instanced                
                Push push{};
                push.mvp = camera->getProjection() * camera->getView() * glm::mat4{ 1.0f };
                Toki::RendererCommand::setConstant(shader, sizeof(Push), &push);

                Toki::RendererCommand::setUniform(shader, uniformBufferLightData, 0, 0, 0);
                Toki::RendererCommand::setUniform(shader, uniformBufferLights, 1, 0, 0);
                Toki::RendererCommand::setTexture(shader, texture, 0, 0, 1);

                Toki::RendererCommand::bindSets(shader);

                Toki::RendererCommand::drawInstanced({ vertexBuffer, instanceBuffer }, indexBuffer, nInstancesX * nInstancesY);

                return;
            }
        }
    }

    void resetBuffers() {
        float halfTotalDistanceX = nInstancesX > 1 ? (nInstancesX - 1) * instanceDistances[0] / 2 : 0;
        float halfTotalDistanceY = nInstancesY > 1 ? (nInstancesY - 1) * instanceDistances[1] / 2 : 0;

        auto modelData = Toki::ModelLoader::loadFromObj(modelPath);

        uint32_t totalModels = nInstancesX * nInstancesY;

        totalBufferMemorySize = 0;

        switch (rendering) {
            case 0: { // Single call
                Toki::VertexBufferConfig vertexBufferConfig{};
                vertexBufferConfig.binding = 0;
                vertexBufferConfig.size = modelData.vertexData.size() * sizeof(Toki::Vertex);
                vertexBuffer = Toki::VertexBuffer::create(vertexBufferConfig);
                vertexBuffer->setData(vertexBufferConfig.size, modelData.vertexData.data());

                totalBufferMemorySize += vertexBufferConfig.size;

                Toki::IndexBufferConfig indexBufferConfig{};
                indexBufferConfig.indexCount = modelData.indexData.size();
                indexBufferConfig.size = modelData.indexData.size() * sizeof(uint32_t);
                indexBuffer = Toki::IndexBuffer::create(indexBufferConfig);
                indexBuffer->setData(indexBufferConfig.size, modelData.indexData.data());

                totalBufferMemorySize += indexBufferConfig.size;

                Toki::VertexBufferConfig instanceBufferConfig{};
                instanceBufferConfig.binding = 1;
                instanceBufferConfig.size = sizeof(InstanceData);
                instanceBuffer = Toki::VertexBuffer::create(instanceBufferConfig);

                InstanceData instance{};
                instanceBuffer->setData(sizeof(InstanceData), &instance);

                totalBufferMemorySize += instanceBufferConfig.size;

                return;
            }

            case 1: { // Batch rendering
                Toki::VertexBufferConfig vertexBufferConfig{};
                vertexBufferConfig.binding = 0;
                vertexBufferConfig.size = modelData.vertexData.size() * sizeof(Toki::Vertex) * totalModels;
                vertexBuffer = Toki::VertexBuffer::create(vertexBufferConfig);
                std::vector<Toki::Vertex> vertexData(modelData.vertexData.size() * totalModels);

                totalBufferMemorySize += vertexBufferConfig.size;

                Toki::IndexBufferConfig indexBufferConfig{};
                indexBufferConfig.indexCount = modelData.indexData.size() * totalModels;
                indexBufferConfig.size = modelData.indexData.size() * sizeof(uint32_t) * totalModels;
                indexBuffer = Toki::IndexBuffer::create(indexBufferConfig);
                std::vector<uint32_t> indexData(modelData.indexData.size() * totalModels);

                totalBufferMemorySize += indexBufferConfig.size;

                Toki::VertexBufferConfig instanceBufferConfig{};
                instanceBufferConfig.binding = 1;
                instanceBufferConfig.size = sizeof(InstanceData);
                instanceBuffer = Toki::VertexBuffer::create(instanceBufferConfig);

                InstanceData instance{};
                instanceBuffer->setData(sizeof(InstanceData), &instance);

                totalBufferMemorySize += instanceBufferConfig.size;

                uint32_t lastVertex = 0;
                uint32_t lastIndex = 0;

                for (uint32_t y = 0; y < nInstancesY; ++y) {
                    for (uint32_t x = 0; x < nInstancesX; ++x) {
                        glm::vec3 position = { x * instanceDistances[0] - halfTotalDistanceX, 0.0f, y * instanceDistances[1] - halfTotalDistanceY };

                        for (uint32_t i = 0; i < modelData.vertexData.size(); ++i) {
                            vertexData[lastVertex + i] = modelData.vertexData[i];
                            vertexData[lastVertex + i].position += position;
                        }

                        for (uint32_t i = 0; i < modelData.indexData.size(); ++i) {
                            indexData[lastIndex + i] = modelData.indexData[i] + lastVertex;
                        }

                        lastVertex += modelData.vertexData.size();
                        lastIndex += modelData.indexData.size();
                    }
                }

                vertexBuffer->setData(vertexBufferConfig.size, vertexData.data());
                indexBuffer->setData(indexBufferConfig.size, indexData.data());

                return;
            }
            case 2: { // Instanced
                Toki::VertexBufferConfig vertexBufferConfig{};
                vertexBufferConfig.binding = 0;
                vertexBufferConfig.size = modelData.vertexData.size() * sizeof(Toki::Vertex);
                vertexBuffer = Toki::VertexBuffer::create(vertexBufferConfig);
                vertexBuffer->setData(vertexBufferConfig.size, modelData.vertexData.data());

                totalBufferMemorySize += vertexBufferConfig.size;

                Toki::IndexBufferConfig indexBufferConfig{};
                indexBufferConfig.indexCount = modelData.indexData.size();
                indexBufferConfig.size = modelData.indexData.size() * sizeof(uint32_t);
                indexBuffer = Toki::IndexBuffer::create(indexBufferConfig);
                indexBuffer->setData(indexBufferConfig.size, modelData.indexData.data());

                totalBufferMemorySize += indexBufferConfig.size;

                Toki::VertexBufferConfig instanceBufferConfig{};
                instanceBufferConfig.binding = 1;
                instanceBufferConfig.size = totalModels * sizeof(InstanceData);
                instanceBuffer = Toki::VertexBuffer::create(instanceBufferConfig);

                totalBufferMemorySize += instanceBufferConfig.size;

                InstanceData* instances = new InstanceData[totalModels]();

                for (uint32_t y = 0; y < nInstancesY; ++y) {
                    for (uint32_t x = 0; x < nInstancesX; ++x) {
                        uint16_t instanceIndex = x + y * nInstancesX;
                        instances[instanceIndex].position = { x * instanceDistances[0] - halfTotalDistanceX, 0.0f, y * instanceDistances[1] - halfTotalDistanceY };
                        instances[instanceIndex].id = (1 << 16) | instanceIndex;
                    }
                }

                instanceBuffer->setData(instanceBufferConfig.size, instances);

                delete[] instances;

                return;
            }
        }
    }

    void onEvent(Toki::Event& event) override {
        if (event.getType() == Toki::EventType::KeyPress) {
            Toki::KeyPressEvent* e = (Toki::KeyPressEvent*) &event;
            std::cout << std::format("Key: {}, Scancode: {}\n", e->getKey(), e->getScancode());
        }
    }

private:
    Toki::Ref<Toki::Shader> shader;
    Toki::Ref<Toki::Framebuffer> framebuffer;
    Toki::Ref<Toki::Framebuffer> testFramebuffer;
    Toki::Ref<Toki::VertexBuffer> vertexBuffer;
    Toki::Ref<Toki::VertexBuffer> instanceBuffer;
    Toki::Ref<Toki::UniformBuffer> uniformBufferLightData;
    Toki::Ref<Toki::UniformBuffer> uniformBufferLights;
    Toki::Ref<Toki::IndexBuffer> indexBuffer;


    Toki::Ref<Toki::VertexBuffer> singleQuadVertexBuffer;
    Toki::Ref<Toki::IndexBuffer> singleQuadIndexBuffer;
    Toki::Ref<Toki::Shader> singleQuadShader;
    Toki::Ref<Toki::UniformBuffer> lighthting;
    float brightness = 1.0f;

    Toki::Ref<Toki::Shader> outlineShader;

    Toki::Ref<Toki::Texture> texture;

    Toki::Ref<Toki::Geometry> model;

    Toki::Ref<Toki::CameraController> camera;
};

int main(int argc, char** argv) {
    Toki::EngineConfig config{};
    config.windowConfig.resizable = false;
    config.workingDirectory = std::filesystem::absolute(argc == 1 ? std::filesystem::path(".") : argv[1]);


    Toki::Engine app(config);

    app.pushLayer(Toki::createRef<TempLayer>());

    app.run();

    return 0;
}