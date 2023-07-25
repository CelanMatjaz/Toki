#include "tkpch.h"

#include "prog_layer.h"

int main(int argc, const char** argv) {
    srand(time(0));
    Toki::Application app(Toki::RendererAPI::VULKAN);
    app.addLayer(new ProgLayer());
    app.run();

    return 0;
}