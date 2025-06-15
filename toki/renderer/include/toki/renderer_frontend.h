#pragma once

#include <toki/core.h>

#include "renderer_commands.h"
#include "renderer_types.h"

namespace toki {

template <typename F>
concept SubmitFunctionConcept = requires(F fn, RendererCommands& commands) {
	{ fn(commands) } -> SameAsConcept<void>;
};

class RendererFrontend {
public:
	struct Config {};

	RendererFrontend(const Config& config = {});
	~RendererFrontend();

	DELETE_COPY(RendererFrontend);
	DELETE_MOVE(RendererFrontend);

public:
	void frame_begin();
	void frame_end();
	void present();

	void submit(SubmitFunctionConcept auto submit_fn);

	void window_add(Window* window);

	Framebuffer framebuffer_create(const FramebufferConfig& config);
	void framebuffer_destroy(Framebuffer& framebuffer);
	void framebuffer_resize(const Framebuffer& framebuffer, u32 width, u32 height);

	Buffer buffer_create(const BufferConfig& config);
	void buffer_destroy(Buffer& buffer);
	void buffer_set_data(const Buffer& buffer, u32 size, void* data);

	Texture texture_create(const TextureConfig& config);
	void texture_destroy(Texture& texture);

	Shader shader_create(Framebuffer framebuffer, const ShaderConfig& config);
	void shader_destroy(Shader& shader);

	void wait_for_resources();

	void set_color_clear(const Vec4<f32>& color);
	void set_depth_clear(f32 depth);

protected:
	void* m_backend;
};

}  // namespace toki
