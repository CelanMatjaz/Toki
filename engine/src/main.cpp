#include "toki.h"

class TestLayer : public Toki::Layer {
public:
    TestLayer() : Toki::Layer(){};
    ~TestLayer() = default;

    void onAttach() override {
        {
            Toki::RenderPassConfig config;
            config.width = 800;
            config.height = 600;

            config.attachments.resize(1);
            Toki::Attachment& attachment = config.attachments[0];
            attachment.colorFormat = Toki::ColorFormat::COLOR_FORMAT_RGBA;
            attachment.loadOp = Toki::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = Toki::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;
            attachment.presentable = true;

            renderPass = Toki::RenderPass::create(config);
        }
    }

    void onRender() override {
        submit(renderPass, [](const Toki::RenderingContext ctx) {});
    }

private:
    Toki::Ref<Toki::RenderPass> renderPass;
};

int main() {
    Toki::ApplicationConfig applicationConfig{};

    Toki::Application app{ applicationConfig };

    app.pushLayer(Toki::createRef<TestLayer>());

    app.start();

    return 0;
}
