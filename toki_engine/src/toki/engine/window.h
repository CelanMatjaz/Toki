#pragma once

#include <functional>
#include <string>

#include "core/math_types.h"
#include "events/event.h"
#include "input/input.h"

namespace toki {

class Engine;

class Window {
public:
    struct Config {
        i32 width = 0;
        i32 height = 0;
        std::string title;
        struct {
            b8 resizable : 1 = false;
        } flags;
    };

protected:
    friend Engine;

    using WindowEventDispatchFn = std::function<void(Engine*, Event)>;

    struct InternalConfig : Config {
        Engine* engine_ptr;
        WindowEventDispatchFn event_dispatch_fn;
    };

public:
    static Ref<Window> create(const InternalConfig& config);

    Window() = delete;
    Window(const InternalConfig& config);
    virtual ~Window() = default;

    virtual Vec2 get_dimensions() const = 0;

    virtual b8 should_close() const = 0;
    virtual void* get_handle() const = 0;
    static void poll_events();

    const Ref<Input> get_input() const;

protected:
    Engine* m_enginePtr;
    WindowEventDispatchFn m_eventDispatchFn;
    Ref<Input> m_input;
};

}  // namespace toki
