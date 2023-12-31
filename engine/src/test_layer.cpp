#include "test_layer.h"

#include "iostream"

TestLayer::TestLayer() {}

TestLayer::~TestLayer() {}

void TestLayer::onAttach() {
    std::cout << "TestLayer Attach\n";
}

void TestLayer::onDetach() {
    std::cout << "TestLayer Detach\n";
}

void TestLayer::onRender() {}

void TestLayer::onUpdate(const float deltaTime) {}

void TestLayer::onEvent(Toki::Event& event) {}
