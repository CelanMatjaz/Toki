#pragma once

#include "core/core.h"

namespace Toki {

    class Renderer {
    public:
        virtual void init() = 0;
        virtual void shutdown() = 0;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

        static void initRenderer();
        static void shutdownRenderer();

        static Ref<Renderer> getRenderer() { return renderer; }

    private:
        static inline bool isInit;
        static Ref<Renderer> renderer;
    };

}