#pragma once

namespace toki {

class RendererApi {
public:
    virtual void reset_viewport() const = 0;
    virtual void reset_scissor() const = 0;
};

}  // namespace toki
