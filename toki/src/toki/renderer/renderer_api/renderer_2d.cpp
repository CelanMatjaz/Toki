#include "renderer_2d.h"

#include "toki/core/assert.h"
#include "toki/core/window.h"
#include "toki/events/event.h"

namespace Toki {

static constexpr const char* QUAD_SHADER_VERT_SOURCE = R"(#version 460
    layout(location = 0) in vec3 inPosition;
    layout(push_constant) uniform PushConstant {
        mat4 mvp;
    } pushConstant;    
    void main() {
        gl_Position = pushConstant.mvp * vec4(inPosition, 1.0);
    })";

static constexpr const char* QUAD_SHADER_FRAG_SOURCE = R"(#version 460
    layout(location = 0) out vec4 outColor;
    layout(set = 0, binding = 0) uniform QuadData {
        vec4 color;
    } quadData;
    void main() {
        outColor = quadData.color;
    })";

struct RendererData {
    struct QuadVertex {
        glm::vec3 position;
    };

    inline static constexpr uint32_t MAX_QUADS = 1000;
    inline static constexpr uint32_t VERTICES_PER_QUAD_VERTEX_BUFFER = 4 * MAX_QUADS;
    inline static constexpr uint32_t INDICES_PER_QUAD_INDEX_BUFFER = 6 * MAX_QUADS;

    bool initialized = false;

    Ref<RenderPass> renderPass;

    OrthographicCamera camera;
    glm::mat4 mvp;

    Ref<Shader> quadShader;
    Ref<VertexBuffer> quadVertexBuffer;
    Ref<IndexBuffer> quadIndexBuffer;
    QuadVertex* quadVertexPtr = nullptr;
    uint32_t* quadIndexPtr = nullptr;
    uint32_t indexCount = 0;

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
        ShaderConfig config{};
        config.attachments = attachments;
        config.shaderStages[ShaderStage::SHADER_STAGE_VERTEX] = (std::string) QUAD_SHADER_VERT_SOURCE;
        config.shaderStages[ShaderStage::SHADER_STAGE_FRAGMENT] = (std::string) QUAD_SHADER_FRAG_SOURCE;
        config.layoutDescriptions.attributeDescriptions = {
            { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, offsetof(RendererData::QuadVertex, position) },
        };
        config.layoutDescriptions.bindingDescriptions = { { 0, sizeof(RendererData::QuadVertex), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX } };
        data.quadShader = Shader::create(config);
    }

    {
        VertexBufferConfig config{};
        config.binding = 0;
        config.size = RendererData::VERTICES_PER_QUAD_VERTEX_BUFFER * sizeof(RendererData::QuadVertex);
        data.quadVertexBuffer = VertexBuffer::create(config);
        data.quadVertexPtr = (RendererData::QuadVertex*) data.quadVertexBuffer->mapMemory(config.size, 0);
    }

    {
        IndexBufferConfig config{};
        config.size = RendererData::INDICES_PER_QUAD_INDEX_BUFFER;
        config.indexCount = RendererData::INDICES_PER_QUAD_INDEX_BUFFER * sizeof(uint32_t);
        config.indexSize = Toki::IndexSize::INDEX_SIZE_32;
        data.quadIndexBuffer = IndexBuffer::create(config);
        data.quadIndexPtr = (uint32_t*) data.quadIndexBuffer->mapMemory(config.size, 0);
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
    if (data.indexCount) {
        submit(data.renderPass, [](const Toki::RenderingContext& ctx) {
            ctx.bindShader(data.quadShader);
            ctx.pushConstants(data.quadShader, sizeof(data.mvp), &data.mvp);
            ctx.bindUniforms(data.quadShader, 0, 1);
            ctx.bindVertexBuffers({ data.quadVertexBuffer });
            ctx.bindIndexBuffer(data.quadIndexBuffer);
            ctx.drawIndexed(data.indexCount, 1, 0, 0, 0);
        });

        data.indexCount = 0;
    }
}

void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    uint32_t currentQuadIndex = data.indexCount / 1.5f;
    data.quadVertexPtr[currentQuadIndex + 0].position = -glm::vec3(position, 0.0f);
    data.quadVertexPtr[currentQuadIndex + 1].position = -glm::vec3(position.x + size.x, position.y, 0.0f);
    data.quadVertexPtr[currentQuadIndex + 2].position = -glm::vec3(position.x, position.y + size.y, 0.0f);
    data.quadVertexPtr[currentQuadIndex + 3].position = -glm::vec3(position + size, 0.0f);

    uint32_t currentIndexIndex = data.indexCount;
    data.quadIndexPtr[currentIndexIndex + 0] = currentIndexIndex + 0;
    data.quadIndexPtr[currentIndexIndex + 1] = currentIndexIndex + 1;
    data.quadIndexPtr[currentIndexIndex + 2] = currentIndexIndex + 2;
    data.quadIndexPtr[currentIndexIndex + 3] = currentIndexIndex + 2;
    data.quadIndexPtr[currentIndexIndex + 4] = currentIndexIndex + 1;
    data.quadIndexPtr[currentIndexIndex + 5] = currentIndexIndex + 3;

    data.indexCount += 6;
}

}  // namespace Toki
