#include "toki.h"

int main() {
    Toki::ApplicationConfig applicationConfig{};

    Toki::Application app{ applicationConfig };
    app.start();

    return 0;
}
