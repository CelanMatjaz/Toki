#include "renderer_2d.h"

#include "toki/core/assert.h"
#include "toki/core/window.h"
#include "toki/events/event.h"

namespace Toki {

static constexpr const char* QUAD_SHADER_VERT_SOURCE = R"(#version 460
    layout (location = 0) out vec4 outColor;
    
    layout (location = 0) in vec3 inPosition;
    layout (location = 1) in vec2 inPositionInstance;
    layout (location = 2) in vec2 inSizeInstance;
    layout (location = 3) in vec4 inColor;

    layout (push_constant) uniform PushConstant {
        mat4 mvp;
    } pushConstant;

    void main() {
        gl_Position = pushConstant.mvp * vec4(inPosition.x * inSizeInstance.x + inPositionInstance.x, inPosition.y * inSizeInstance.y + inPositionInstance.y, inPosition.z, 1.0);
        outColor = inColor;
    })";

static constexpr const char* QUAD_SHADER_FRAG_SOURCE = R"(#version 460
    layout (location = 0) out vec4 outColor;

    layout (location = 0) in vec4 inColor;

    layout (set = 0, binding = 0) uniform QuadData {
        vec4 color;
    } quadData;
    void main() {
        outColor = inColor;
    })";

struct RendererData {
    struct QuadVertex {
        glm::vec3 position;
    };

    struct QuadInstance {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec4 color;
    };

    inline static constexpr uint32_t MAX_QUADS = 1000;

    bool initialized = false;

    Ref<RenderPass> renderPass;

    OrthographicCamera camera;
    glm::mat4 mvp;

    Ref<Shader> quadShader;
    Ref<VertexBuffer> quadVertexBuffer;
    Ref<VertexBuffer> quadInstanceVertexBuffer;
    Ref<IndexBuffer> quadIndexBuffer;
    QuadInstance* quadInstancePtr = nullptr;
    uint32_t instanceCount = 0;

    Ref<UniformBuffer> colorUniformBuffer;
};

static RendererData data;

void Renderer2D::init(Ref<Window> window) {
    TK_ASSERT(!data.initialized, "Renderer2D already initialized");
    initObjects(window);
    data.initialized = true;
}

void Renderer2D::shutdown() {
    TK_ASSERT(data.initialized, "Renderer2D not initialized");

    Event::unbindEvent(EventType::WindowResize, &data);
    data = {};
}

void Renderer2D::begin() {}

void Renderer2D::end() {
    flush();
}

void Renderer2D::submit(Ref<RenderPass> renderpass, RendererSubmitFn submitFn) {
    s_renderer->submit(renderpass, submitFn);
}

void Renderer2D::initObjects(Ref<Window> window) {
    const auto [width, height] = window->getDimensions();

    std::vector<Attachment> attachments(2);

    Attachment& presentAttachment = attachments[0] = {};
    presentAttachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_RGBA;
    presentAttachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD;
    presentAttachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;
    presentAttachment.presentable = true;

    Attachment& depthAttachment = attachments[1] = {};
    depthAttachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_DEPTH_STENCIL;
    depthAttachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;

    {
        RenderPassConfig config{};
        config.attachments = attachments;
        config.width = width;
        config.height = height;
        data.renderPass = RenderPass::create(config);
    }

    {
        Toki::ShaderOptions options{};
        options.primitiveTopology = Toki::PrimitiveTopology::TriangleList;
        options.polygonMode = Toki::PolygonMode::Fill;
        options.cullMode = Toki::CullMode::Back;
        options.frontFace = Toki::FrontFace::Clockwise;
        options.depthTest.enable = true;
        options.depthTest.write = true;
        options.depthTest.compareOp = Toki::CompareOp::GreaterOrEqual;

        ShaderConfig config{};
        config.options = options;
        config.attachments = attachments;
        config.shaderStages[ShaderStage::SHADER_STAGE_VERTEX] = (std::string) QUAD_SHADER_VERT_SOURCE;
        config.shaderStages[ShaderStage::SHADER_STAGE_FRAGMENT] = (std::string) QUAD_SHADER_FRAG_SOURCE;
        config.layoutDescriptions.attributeDescriptions = {
            { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, offsetof(RendererData::QuadVertex, position) },
            { 1, 1, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, offsetof(RendererData::QuadInstance, position) },
            { 2, 1, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, offsetof(RendererData::QuadInstance, size) },
            { 3, 1, Toki::VertexFormat::VERTEX_FORMAT_FLOAT4, offsetof(RendererData::QuadInstance, color) },
        };
        config.layoutDescriptions.bindingDescriptions = {
            { 0, sizeof(RendererData::QuadVertex), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX },
            { 1, sizeof(RendererData::QuadInstance), Toki::VertexInputRate::VERTEX_INPUT_RATE_INSTANCE }
        };
        data.quadShader = Shader::create(config);
    }

    {
        uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };

        IndexBufferConfig config{};
        config.size = sizeof(indices);
        config.indexCount = 6;
        config.indexSize = Toki::IndexSize::INDEX_SIZE_16;
        data.quadIndexBuffer = IndexBuffer::create(config);
        data.quadIndexBuffer->setData(sizeof(indices), indices);
    }

    {
        RendererData::QuadVertex vertices[] = {
            glm::vec3{ 0.0f, 0.0f, 0.0f },
            glm::vec3{ 1.0f, 0.0f, 0.0f },
            glm::vec3{ 0.0f, 1.0f, 0.0f },
            glm::vec3{ 1.0f, 1.0f, 0.0f },
        };

        VertexBufferConfig config{};
        config.binding = 0;
        config.size = sizeof(vertices);
        data.quadVertexBuffer = VertexBuffer::create(config);
        data.quadVertexBuffer->setData(sizeof(vertices), vertices);
    }

    {
        VertexBufferConfig config{};
        config.binding = 1;
        config.size = RendererData::MAX_QUADS * sizeof(RendererData::QuadInstance);
        data.quadInstanceVertexBuffer = VertexBuffer::create(config);
        data.quadInstancePtr = (RendererData::QuadInstance*) data.quadInstanceVertexBuffer->mapMemory(config.size, 0);
    }

    {
        UniformBufferConfig config{};
        config.size = sizeof(glm::vec4);
        data.colorUniformBuffer = UniformBuffer::create(config);
        glm::vec4 defaultColor = glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f };
        data.colorUniformBuffer->setData(sizeof(glm::vec4), &defaultColor);
        data.quadShader->setUniforms({ { data.colorUniformBuffer } });
    }

    data.camera.setProjection(glm::ortho(0.0f, (float) width, 0.0f, (float) height, -10.0f, 10.0f));
    data.camera.setView(glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }));
    data.mvp = data.camera.getProjection() * data.camera.getView();

    Event::bindEvent(EventType::WindowResize, &data, [](void* sender, void* receiver, const Toki::Event& event) {
        EventData eventData = event.getData();
        data.camera.setProjection(glm::ortho(0.0f, (float) eventData.i32[0], 0.0f, (float) eventData.i32[1], -10.0f, 10.0f));
        data.mvp = data.camera.getProjection() * data.camera.getView();
    });
}

void Renderer2D::flush() {
    if (data.instanceCount > 0) {
        submit(data.renderPass, [](const Toki::RenderingContext& ctx) {
            ctx.bindShader(data.quadShader);
            ctx.pushConstants(data.quadShader, sizeof(data.mvp), &data.mvp);
            ctx.bindUniforms(data.quadShader, 0, 1);
            ctx.bindVertexBuffers({ data.quadVertexBuffer, data.quadInstanceVertexBuffer });
            ctx.bindIndexBuffer(data.quadIndexBuffer);
            ctx.drawIndexed(6, data.instanceCount, 0, 0, 0);
        });

        data.instanceCount = 0;
    }
}

void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    uint32_t currentInstanceIndex = data.instanceCount;
    RendererData::QuadInstance& currentInstance = data.quadInstancePtr[currentInstanceIndex];

    currentInstance.position = -position;
    currentInstance.size = -size;
    currentInstance.color = color;

    ++data.instanceCount;
}

void Renderer2D::drawQuad(const Quad& quad) {
    uint32_t currentInstanceIndex = data.instanceCount;
    RendererData::QuadInstance& currentInstance = data.quadInstancePtr[currentInstanceIndex];

    currentInstance.position = -quad.position;
    currentInstance.size = -quad.size;
    currentInstance.color = quad.color;

    ++data.instanceCount;
}

void Renderer2D::drawQuads(std::vector<Quad> quads) {
    for (const auto& quad : quads) {
        drawQuad(quad);
    }
}

}  // namespace Toki
