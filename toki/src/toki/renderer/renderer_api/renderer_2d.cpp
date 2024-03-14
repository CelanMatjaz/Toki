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

static constexpr const char* TEXT_SHADER_VERT_SOURCE = R"(#version 460
    layout (location = 0) out vec2 outUV;
    
    layout (location = 0) in vec3 inPosition;
    layout (location = 1) in vec2 inUV;

    layout (push_constant) uniform PushConstant {
        mat4 mvp;
    } pushConstant;

    void main() {
        gl_Position = pushConstant.mvp * vec4(inPosition, 1.0);
        outUV = inUV;
    })";

static constexpr const char* TEXT_SHADER_FRAG_SOURCE = R"(#version 460
    layout (location = 0) out vec4 outColor;

    layout (location = 0) in vec2 inUV;
   
    layout(set = 0, binding = 0) uniform texture2D textureIn;
    layout(set = 0, binding = 1) uniform sampler samplerIn;

    void main() {
        outColor =  texture(sampler2D(textureIn, samplerIn), vec2(inUV.x , inUV.y));
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

    struct CharacterVertex {
        glm::vec3 position;
        glm::vec2 uv;
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

    Ref<Shader> textShader;
    Ref<VertexBuffer> textVertexBuffer;
    Ref<IndexBuffer> characterIndexBuffer;
    CharacterVertex* characterInstancePtr = nullptr;
    uint32_t* characterIndexPtr = nullptr;
    uint32_t characterVertexCount = 0;
    uint32_t characterIndexCount = 0;

    Ref<UniformBuffer> colorUniformBuffer;
    Ref<Sampler> textSampler;
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

        config.shaderStages[ShaderStage::SHADER_STAGE_VERTEX] = (std::string) TEXT_SHADER_VERT_SOURCE;
        config.shaderStages[ShaderStage::SHADER_STAGE_FRAGMENT] = (std::string) TEXT_SHADER_FRAG_SOURCE;
        config.layoutDescriptions.attributeDescriptions = {
            { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, offsetof(RendererData::CharacterVertex, position) },
            { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, offsetof(RendererData::CharacterVertex, uv) },
        };
        config.layoutDescriptions.bindingDescriptions = {
            { 0, sizeof(RendererData::CharacterVertex), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX },
        };
        data.textShader = Shader::create(config);
    }

    {
        Toki::SamplerConfig fontSamplerConfig{};
        fontSamplerConfig.repeatU = fontSamplerConfig.repeatV = fontSamplerConfig.repeatW = TextureRepeat::ClampEdge;
        fontSamplerConfig.magFilter = fontSamplerConfig.minFilter = TextureFilter::Nearest;
        data.textSampler = Toki::Sampler::create(fontSamplerConfig);
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
        IndexBufferConfig config{};
        config.size = 6 * RendererData::MAX_QUADS * 6;
        config.indexCount = 6 * RendererData::MAX_QUADS;
        config.indexSize = Toki::IndexSize::INDEX_SIZE_32;
        data.characterIndexBuffer = IndexBuffer::create(config);
        data.characterIndexPtr = (uint32_t*) data.characterIndexBuffer->mapMemory(6 * RendererData::MAX_QUADS * 6, 0);
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
        VertexBufferConfig config{};
        config.binding = 1;
        config.size = RendererData::MAX_QUADS * sizeof(RendererData::CharacterVertex) * 6;
        data.textVertexBuffer = VertexBuffer::create(config);
        data.characterInstancePtr = (RendererData::CharacterVertex*) data.textVertexBuffer->mapMemory(config.size, 0);
    }

    {
        UniformBufferConfig config{};
        config.size = sizeof(glm::vec4);
        data.colorUniformBuffer = UniformBuffer::create(config);
        glm::vec4 defaultColor = glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f };
        data.colorUniformBuffer->setData(sizeof(glm::vec4), &defaultColor);
        data.quadShader->setUniforms({ { data.colorUniformBuffer } });
    }

    data.textShader->setUniforms({ { data.textSampler, 0, 1 } });

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
    submit(data.renderPass, [](const Toki::RenderingContext& ctx) {
        if (data.instanceCount > 0) {
            ctx.bindShader(data.quadShader);
            ctx.pushConstants(data.quadShader, sizeof(data.mvp), &data.mvp);
            ctx.bindUniforms(data.quadShader, 0, 1);
            ctx.bindVertexBuffers({ data.quadVertexBuffer, data.quadInstanceVertexBuffer });
            ctx.bindIndexBuffer(data.quadIndexBuffer);
            ctx.drawIndexed(6, data.instanceCount, 0, 0, 0);

            data.instanceCount = 0;
        }

        if (data.characterIndexCount > 0) {
            ctx.bindShader(data.textShader);
            ctx.pushConstants(data.textShader, sizeof(data.mvp), &data.mvp);
            ctx.bindUniforms(data.textShader, 0, 1);
            ctx.bindVertexBuffers({ data.textVertexBuffer });
            ctx.bindIndexBuffer(data.characterIndexBuffer);
            ctx.drawIndexed(data.characterIndexCount, 1, 0, 0, 0);

            data.characterIndexCount = 0;
            data.characterVertexCount = 0;
        }
    });
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

void Renderer2D::drawFont(const glm::vec2& position, Ref<FontData> font, std::string text) {
    data.textShader->setUniforms({ { font->atlas, 0, 0 }, { data.textSampler, 0, 1 } });

    glm::vec2 offset = position;
    offset.y += 13;

    for (const auto& c : text) {
        auto& g = font->glyphs[c - 31];

        data.characterInstancePtr[data.characterVertexCount + 0] = { -glm::vec3{ offset.x, offset.y + g.yOffset, 0.0 },
                                                                     (glm::vec2{ g.x, g.y } / 1024.0f) };
        data.characterInstancePtr[data.characterVertexCount + 1] = { -glm::vec3{ offset.x + g.width, offset.y + g.yOffset, 0.0 },
                                                                     (glm::vec2{ g.x + g.width, g.y } / 1024.0f) };
        data.characterInstancePtr[data.characterVertexCount + 2] = { -glm::vec3{ offset.x, offset.y + g.height + g.yOffset, 0.0 },
                                                                     (glm::vec2{ g.x, g.y + g.height } / 1024.0f) };
        data.characterInstancePtr[data.characterVertexCount + 3] = { -glm::vec3{ offset.x + g.width, offset.y + g.height + g.yOffset, 0.0 },
                                                                     (glm::vec2{ g.x + g.width, g.y + g.height } / 1024.0f) };

        data.characterIndexPtr[data.characterIndexCount + 0] = data.characterVertexCount + 0;
        data.characterIndexPtr[data.characterIndexCount + 1] = data.characterVertexCount + 1;
        data.characterIndexPtr[data.characterIndexCount + 2] = data.characterVertexCount + 2;
        data.characterIndexPtr[data.characterIndexCount + 3] = data.characterVertexCount + 2;
        data.characterIndexPtr[data.characterIndexCount + 4] = data.characterVertexCount + 1;
        data.characterIndexPtr[data.characterIndexCount + 5] = data.characterVertexCount + 3;

        offset += glm::vec2{ g.width + 1, 0 };

        data.characterVertexCount += 4;
        data.characterIndexCount += 6;
    }

    // exit(-1);
}

}  // namespace Toki
