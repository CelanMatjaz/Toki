#include "test_layer.h"

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
}

void TestLayer::onDetach() {
    Toki::Renderer& renderer = Application::getRenderer();

    renderer.destroyBuffer(m_vertexBuffer);
    renderer.destroyBuffer(m_indexBuffer);
    renderer.destroyShader(m_shader);
}

void TestLayer::onRender() {
    Renderer& renderer = Application::getRenderer();

    renderer.resetViewport();
    renderer.resetScissor();

    renderer.bindIndexBuffer(m_indexBuffer);
    renderer.bindVertexBuffers({ { m_vertexBuffer, 0, 0 } });

    renderer.bindFramebuffer(m_framebuffer);

    renderer.bindShader(m_shader);
    renderer.setTextureWithSampler(m_testTexture, m_testSampler, 0, 0, 0);
    renderer.bindUniforms(0, 1);
    renderer.drawIndexed(6, 1, 0, 0, 0);

    renderer.bindShader(m_shader2);
    renderer.setTexture(m_testTexture, 0, 0, 0);
    renderer.setSampler(m_testSampler, 0, 1, 0);
    renderer.bindUniforms(0, 1);
    renderer.drawIndexed(6, 1, 0, 0, 0);

    renderer.unbindFramebuffer();
}

void TestLayer::onEvent(Toki::Event& event) {}
