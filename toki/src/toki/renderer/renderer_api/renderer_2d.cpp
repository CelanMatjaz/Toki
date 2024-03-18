#include "renderer_2d.h"

#include "toki/core/assert.h"
#include "toki/core/window.h"
#include "toki/events/event.h"

namespace Toki {

static constexpr const char* QUAD_SHADER_VERT_SOURCE = R"(#version 460
    layout (location = 0) out vec2 outUV;
    layout (location = 1) out vec4 outColor;
    
    layout (location = 0) in vec3 inPosition;
    layout (location = 1) in vec2 inUV;
    layout (location = 2) in vec4 inColor;

    layout (push_constant) uniform PushConstant {
        mat4 mvp;
    } pushConstant;

    void main() {
        gl_Position = pushConstant.mvp * vec4(-inPosition, 1.0);
        outUV = inUV;
        outColor = inColor;
    })";

static constexpr const char* QUAD_SHADER_FRAG_SOURCE = R"(#version 460
    layout (location = 0) out vec4 outColor;

    layout (location = 0) in vec2 inUV;
    layout (location = 1) in vec4 inColor;
   
    layout(set = 0, binding = 0) uniform texture2D textureIn;
    layout(set = 0, binding = 1) uniform sampler samplerIn;

    void main() {
        if(inColor.w == 0) {
            outColor = texture(sampler2D(textureIn, samplerIn), vec2(inUV.x , inUV.y)).xyzw;
        } else {
            outColor = inColor;
        }
    })";

struct RendererData {
    struct QuadBVertex {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec4 color;
    };

    inline static constexpr uint32_t MAX_QUADS = 1000;

    bool initialized = false;

    Ref<RenderPass> renderPass;

    OrthographicCamera camera;
    glm::mat4 mvp;

    Ref<Shader> batchShader;
    Ref<VertexBuffer> batchQuadVertexBuffer;
    Ref<IndexBuffer> batchQuadIndexBuffer;
    QuadBVertex batchQuads[MAX_QUADS * 4];
    uint32_t batchQuadIndices[MAX_QUADS * 6];
    uint32_t batchQuadCount = 0;

    Ref<Sampler> textSampler;
    Ref<Texture> defaultTexture;
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
        Toki::GraphicsShaderOptions options{};
        options.primitiveTopology = Toki::PrimitiveTopology::TriangleList;
        options.polygonMode = Toki::PolygonMode::Fill;
        options.cullMode = Toki::CullMode::Back;
        options.frontFace = Toki::FrontFace::Clockwise;
        options.depthTest.enable = false;
        options.depthTest.write = true;
        options.depthTest.compareOp = Toki::CompareOp::LessOrEqual;
        options.attachments = attachments;
        options.layoutDescriptions.attributeDescriptions = {
            { 0, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT3, offsetof(RendererData::QuadBVertex, position) },
            { 1, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT2, offsetof(RendererData::QuadBVertex, uv) },
            { 2, 0, Toki::VertexFormat::VERTEX_FORMAT_FLOAT4, offsetof(RendererData::QuadBVertex, color) },
        };
        options.layoutDescriptions.bindingDescriptions = {
            { 0, sizeof(RendererData::QuadBVertex), Toki::VertexInputRate::VERTEX_INPUT_RATE_VERTEX },
        };

        ShaderConfig config{};
        config.options = options;
        config.shaderStages[ShaderStage::SHADER_STAGE_VERTEX] = (std::string) QUAD_SHADER_VERT_SOURCE;
        config.shaderStages[ShaderStage::SHADER_STAGE_FRAGMENT] = (std::string) QUAD_SHADER_FRAG_SOURCE;

        data.batchShader = Shader::create(config);
    }

    {
        Toki::SamplerConfig fontSamplerConfig{};
        fontSamplerConfig.repeatU = fontSamplerConfig.repeatV = fontSamplerConfig.repeatW = TextureRepeat::ClampEdge;
        fontSamplerConfig.magFilter = fontSamplerConfig.minFilter = TextureFilter::Nearest;
        data.textSampler = Toki::Sampler::create(fontSamplerConfig);
    }

    {
        uint32_t color = 0xFFFF00FF;

        TextureConfig config{};
        config.width = 1;
        config.height = 1;
        data.defaultTexture = Texture::create(config);
        data.defaultTexture->setData(sizeof(color), &color);
    }

    {
        IndexBufferConfig config{};
        config.size = sizeof(uint32_t) * RendererData::MAX_QUADS;
        config.indexSize = Toki::IndexSize::INDEX_SIZE_32;
        data.batchQuadIndexBuffer = IndexBuffer::create(config);
    }

    {
        VertexBufferConfig config{};
        config.binding = 0;
        config.size = sizeof(RendererData::QuadBVertex) * RendererData::MAX_QUADS;
        data.batchQuadVertexBuffer = VertexBuffer::create(config);
    }

    data.batchShader->setUniforms({ { data.defaultTexture, 0, 0 }, { data.textSampler, 0, 1 } });

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
    if (data.batchQuadCount > 0) {
        data.batchQuadVertexBuffer->setData(sizeof(RendererData::QuadBVertex) * data.batchQuadCount, data.batchQuads);
        data.batchQuadIndexBuffer->setData(data.batchQuadCount * 6, data.batchQuadIndices);
    }

    submit(data.renderPass, [](const Toki::RenderingContext& ctx) {
        if (data.batchQuadCount > 0) {
            ctx.bindShader(data.batchShader);
            ctx.pushConstants(data.batchShader, sizeof(data.mvp), &data.mvp);
            ctx.bindUniforms(data.batchShader, 0, 1);
            ctx.bindVertexBuffers({ data.batchQuadVertexBuffer });
            ctx.bindIndexBuffer(data.batchQuadIndexBuffer);
            ctx.drawIndexed(data.batchQuadCount * 1.5, 1, 0, 0, 0);

            data.batchQuadCount = 0;
        }
    });
}

void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    uint32_t currentQuadIndex = data.batchQuadCount;

    data.batchQuads[currentQuadIndex + 0] = RendererData::QuadBVertex{ { position.x, position.y, 0.0f }, { 0.0f, 0.0f }, color };
    data.batchQuads[currentQuadIndex + 1] = RendererData::QuadBVertex{ { position.x + size.x, position.y, 0.0f }, { 0.0f, 0.0f }, color };
    data.batchQuads[currentQuadIndex + 2] = RendererData::QuadBVertex{ { position.x, position.y + size.y, 0.0f }, { 0.0f, 0.0f }, color };
    data.batchQuads[currentQuadIndex + 3] = RendererData::QuadBVertex{ { position.x + size.x, position.y + size.y, 0.0f }, { 0.0f, 0.0f }, color };

    uint32_t currentIndexIndex = currentQuadIndex * 1.5;

    data.batchQuadIndices[currentIndexIndex + 0] = currentQuadIndex + 0;
    data.batchQuadIndices[currentIndexIndex + 1] = currentQuadIndex + 1;
    data.batchQuadIndices[currentIndexIndex + 2] = currentQuadIndex + 2;
    data.batchQuadIndices[currentIndexIndex + 3] = currentQuadIndex + 2;
    data.batchQuadIndices[currentIndexIndex + 4] = currentQuadIndex + 1;
    data.batchQuadIndices[currentIndexIndex + 5] = currentQuadIndex + 3;

    data.batchQuadCount += 4;
}

void Renderer2D::drawQuad(const Quad& quad) {
    drawQuad(quad.position, quad.size, quad.color);
}

void Renderer2D::drawQuads(std::vector<Quad> quads) {
    for (const auto& quad : quads) {
        drawQuad(quad);
    }
}

void Renderer2D::drawQuadB(const Quad& quad) {}

void Renderer2D::drawText(const glm::vec2& position, Ref<FontData> font, std::string text) {
    data.batchShader->setUniforms({ { font->atlas, 0, 0 } });

    glm::vec2 offset = position;
    offset.y += 13;

    for (const auto& c : text) {
        if (c == ' ') {
            offset.x += 6;
            continue;
        }

        auto& g = font->glyphs[c - 31];

        uint32_t currentQuadIndex = data.batchQuadCount;

        glm::vec2 del = glm::vec2{ 1024.0f, 1024.0f };

        data.batchQuads[currentQuadIndex + 0] =
            RendererData::QuadBVertex{ { offset.x, offset.y + g.yOffset, 0.0f }, glm::vec2{ g.x, g.y } / del, glm::vec4{ 0.0f } };

        data.batchQuads[currentQuadIndex + 1] =
            RendererData::QuadBVertex{ { offset.x + g.width, offset.y + g.yOffset, 0.0f }, glm::vec2{ g.x + g.width, g.y } / del, glm::vec4{ 0.0f } };

        data.batchQuads[currentQuadIndex + 2] = RendererData::QuadBVertex{ { offset.x, offset.y + g.height + g.yOffset, 0.0f },
                                                                           glm::vec2{ g.x, g.y + g.height } / del,
                                                                           glm::vec4{ 0.0f } };

        data.batchQuads[currentQuadIndex + 3] = RendererData::QuadBVertex{ { offset.x + g.width, offset.y + g.height + g.yOffset, 0.0f },
                                                                           glm::vec2{ g.x + g.width, g.y + g.height } / del,
                                                                           glm::vec4{ 0.0f } };

        uint32_t currentIndexIndex = currentQuadIndex * 1.5;

        data.batchQuadIndices[currentIndexIndex + 0] = currentQuadIndex + 0;
        data.batchQuadIndices[currentIndexIndex + 1] = currentQuadIndex + 1;
        data.batchQuadIndices[currentIndexIndex + 2] = currentQuadIndex + 2;
        data.batchQuadIndices[currentIndexIndex + 3] = currentQuadIndex + 2;
        data.batchQuadIndices[currentIndexIndex + 4] = currentQuadIndex + 1;
        data.batchQuadIndices[currentIndexIndex + 5] = currentQuadIndex + 3;

        data.batchQuadCount += 4;
        offset += glm::vec2{ g.width + 1, 0 };

        // data.characterInstancePtr[data.characterVertexCount + 0] = { -glm::vec3{ offset.x, offset.y + g.yOffset, 0.0 },
        //                                                              (glm::vec2{ g.x, g.y } / 1024.0f) };

        // data.characterInstancePtr[data.characterVertexCount + 1] = { -glm::vec3{ offset.x + g.width, offset.y + g.yOffset, 0.0 },
        //                                                              (glm::vec2{ g.x + g.width, g.y } / 1024.0f) };

        // data.characterInstancePtr[data.characterVertexCount + 2] = { -glm::vec3{ offset.x, offset.y + g.height + g.yOffset, 0.0 },
        //                                                              (glm::vec2{ g.x, g.y + g.height } / 1024.0f) };

        // data.characterInstancePtr[data.characterVertexCount + 3] = { -glm::vec3{ offset.x + g.width, offset.y + g.height + g.yOffset, 0.0 },
        //                                                              (glm::vec2{ g.x + g.width, g.y + g.height } / 1024.0f) };

        // data.characterIndexPtr[data.characterIndexCount + 0] = data.characterVertexCount + 0;
        // data.characterIndexPtr[data.characterIndexCount + 1] = data.characterVertexCount + 1;
        // data.characterIndexPtr[data.characterIndexCount + 2] = data.characterVertexCount + 2;
        // data.characterIndexPtr[data.characterIndexCount + 3] = data.characterVertexCount + 2;
        // data.characterIndexPtr[data.characterIndexCount + 4] = data.characterVertexCount + 1;
        // data.characterIndexPtr[data.characterIndexCount + 5] = data.characterVertexCount + 3;

        // offset += glm::vec2{ g.width + 1, 0 };

        // data.characterVertexCount += 4;
        // data.characterIndexCount += 6;
    }

    // exit(-1);
}

}  // namespace Toki
