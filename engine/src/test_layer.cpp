#include "test_layer.h"

#include "toki/renderer/command.h"
#include "toki/resources/loaders/config_loader.h"

void TestLayer::onAttach() {
    {
        struct Vertex {
            Point3D position;
            Point2D uv;
        };

        Vertex vertices[] = {
            { { +0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
            { { +0.5f, +0.5f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f, +0.5f, 0.0f }, { 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
        };

        m_vertexBuffer = Application::getRenderer().createBuffer(BufferType::VertexBuffer, sizeof(vertices));
        Application::getRenderer().setBufferData(m_vertexBuffer, sizeof(vertices), 0, vertices);
    }

    {
        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        m_indexBuffer = Application::getRenderer().createBuffer(BufferType::IndexBuffer, sizeof(indices));
        Application::getRenderer().setBufferData(m_indexBuffer, sizeof(indices), 0, indices);
    }

    std::vector<Attachment> attachments;

    Attachment presentAttachment{};
    presentAttachment.presentable = true;
    presentAttachment.loadOp = AttachmentLoadOp::Clear;
    presentAttachment.storeOp = AttachmentStoreOp::Store;
    presentAttachment.colorFormat = ColorFormat::RGBA8;
    attachments.emplace_back(presentAttachment);

    {
        FramebufferConfig config{};
        config.attachments = attachments;

        m_framebuffer = Application::getRenderer().createFramebuffer(config);
    }

    {
        ShaderConfig config = ConfigLoader::loadShaderConfig("assets/configs/simple.shader.scfg").value_or(ShaderConfig{});
        config.attachments = attachments;
        m_shader = Application::getRenderer().createShader(config);

        config = ConfigLoader::loadShaderConfig("assets/configs/simple.shader2.scfg").value_or(ShaderConfig{});
        config.attachments = attachments;
        m_shader2 = Application::getRenderer().createShader(config);
    }

    { m_testTexture = Application::getRenderer().createTexture(ColorFormat::RGBA8, "assets/textures/chad.jpg"); }

    { m_testSampler = Application::getRenderer().createSampler(); }

    {
        BufferConfig config;
        config.size = sizeof(float);
        config.type = BufferType::UniformBuffer;
        m_testUniform = Application::getRenderer().createBuffer(config);

        float rotationZ = 50;
        Application::getRenderer().setBufferData(m_testUniform, sizeof(float), 0, &rotationZ);
    }
}

void TestLayer::onDetach() {
    Toki::Renderer& renderer = Application::getRenderer();
}

void TestLayer::onRender() {
    Renderer& renderer = Application::getRenderer();

    renderer.submit([this](Command& cmd) {
        cmd.resetViewport();
        cmd.resetScissor();

        cmd.bindIndexBuffer(m_indexBuffer);
        cmd.bindVertexBuffers({ { m_vertexBuffer, 0, 0 } });

        cmd.bindFramebuffer(m_framebuffer);

        cmd.bindShader(m_shader);
        cmd.setTextureWithSampler(m_testTexture, m_testSampler, 0, 0, 0);
        cmd.bindUniforms(0, 1);
        cmd.drawIndexed(6, 1, 0, 0, 0);

        cmd.bindShader(m_shader2);
        cmd.setTexture(m_testTexture, 0, 0, 0);
        cmd.setSampler(m_testSampler, 0, 1, 0);
        cmd.setUniformBuffer(m_testUniform, 1, 0, 0);
        cmd.bindUniforms(0, 2);
        cmd.drawIndexed(6, 1, 0, 0, 0);

        cmd.unbindFramebuffer();
    });
}

void TestLayer::onEvent(Toki::Event& event) {}
