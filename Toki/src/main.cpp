#include "tkpch.h"

#include "toki/core/toki.h"

class TestApplication : public Toki::Application {
public:
    TestApplication() : Toki::Application() {}
    ~TestApplication() = default;
    
private:
    void onUpdate(float deltaTime) override {

    }
};

int main(int argc, const char** argv) {
    glfwInit();

    TestApplication app;
    app.run();

    glfwTerminate();
    return 0;
}