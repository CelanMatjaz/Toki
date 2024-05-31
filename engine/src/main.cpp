#include "test_layer.h"

int main() {
    Toki::ApplicationConfig applicationConfig{};
    applicationConfig.windowConfig.isResizable = true;
    applicationConfig.windowConfig.showOnCreate = true;

    Toki::Application app{ applicationConfig };
    app.pushLayer(Toki::createRef<TestLayer>());
    app.start();

    return 0;
}
