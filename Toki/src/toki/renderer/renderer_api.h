#pragma once 

namespace Toki {

    class RendererAPI {
public:
    enum API {
        NONE,
        VULKAN,        
    };

public:
        virtual void init() = 0;
        virtual void shutdown() = 0;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

        virtual void onResize() {}
    };

}