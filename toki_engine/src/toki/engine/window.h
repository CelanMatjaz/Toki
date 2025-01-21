#pragma once

#include <functional>
#include <string>

#include "core/math_types.h"
#include "events/event.h"
#include "events/event_handler.h"
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

    struct InternalConfig : Config {};

public:
    static Window* create(const InternalConfig& config);

    Window() = delete;
    Window(const InternalConfig& config);
    virtual ~Window() = default;

    virtual Vec2 get_dimensions() const = 0;

    virtual b8 should_close() const = 0;
    virtual void* get_handle() const = 0;
    static void poll_events();

    const Input& get_input() const;

protected:
    Input* m_input;
    EventHandler m_eventHandler;
};

}  // namespace toki
