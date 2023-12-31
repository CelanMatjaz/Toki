#pragma once

#include "toki.h"

class TestLayer : public Toki::Layer {
public:
    TestLayer();
    virtual ~TestLayer();

    virtual void onAttach() override;

    virtual void onDetach() override;

    virtual void onRender() override;

    virtual void onUpdate(const float deltaTime) override;

    virtual void onEvent(Toki::Event& event) override;
};
