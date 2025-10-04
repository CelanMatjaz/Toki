#pragma once

#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <toki/renderer/commands.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/types.h>

namespace toki::renderer {

struct RendererConfig {
	platform::Window* window;
};

class Renderer {
public:
	static toki::UniquePtr<Renderer> create(const RendererConfig& config);

	Renderer() = delete;
	Renderer(const RendererConfig& config);
	virtual ~Renderer() = default;

	DELETE_COPY(Renderer);
	DELETE_MOVE(Renderer);

	virtual void frame_prepare() = 0;
	virtual void frame_cleanup() = 0;
	virtual Commands* get_commands() = 0;
	virtual void submit(Commands*) = 0;
	virtual void present() = 0;

	// virtual void attach_window(platform::Window* window) = 0;

	virtual ShaderLayoutHandle create_shader_layout(const ShaderLayoutConfig& config) = 0;
	virtual ShaderHandle create_shader(const ShaderConfig& config) = 0;

public:
	virtual void destroy_handle(ShaderHandle) = 0;
	virtual void destroy_handle(ShaderLayoutHandle) = 0;
};

}  // namespace toki::renderer
