#include "toki.h"

using namespace Toki;

class TestLayer : public Toki::Layer {
public:
    TestLayer() : Toki::Layer(){};
    ~TestLayer() = default;

    void onAttach() override;
    void onDetach() override;
    void onRender() override;
    void onEvent(Event& event) override;

private:
    Handle m_vertexBuffer;
    Handle m_indexBuffer;
    Handle m_framebuffer;
    Handle m_shader;
    Handle m_shader2;

    Handle m_testTexture;
    Handle m_testSampler;
};
