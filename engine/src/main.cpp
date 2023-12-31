#include "iostream"
#include "test_layer.h"
#include "toki.h"

int main() {
    std::cout << "Application\n";
    Toki::Application app;
    app.pushLayer(Toki::createRef<TestLayer>());

    app.run();

    return 0;
}
