#pragma once

#include <toki/core/core.h>
#include <toki/renderer/commands.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/types.h>

namespace toki {

struct RendererConfig {
	Window* window;
};

class Renderer {
	friend class Engine;

public:
	static toki::UniquePtr<Renderer> create(const RendererConfig& config);
	Renderer(const RendererConfig& config);

private:
	Renderer() = delete;

	DELETE_COPY(Renderer);
	DELETE_MOVE(Renderer);

	// virtual void attach_window(platform::Window* window) = 0;
public:
	~Renderer() = default;

	ShaderLayoutHandle create_shader_layout(const ShaderLayoutConfig& config);
	ShaderHandle create_shader(const ShaderConfig& config);
	BufferHandle create_buffer(const BufferConfig& config);
	TextureHandle create_texture(const TextureConfig& config);
	SamplerHandle create_sampler(const SamplerConfig& config);

	void destroy_handle(ShaderHandle);
	void destroy_handle(ShaderLayoutHandle);
	void destroy_handle(BufferHandle);
	void destroy_handle(TextureHandle);
	void destroy_handle(SamplerHandle);
	void destroy_handle(CommandsHandle);

	// Buffer functions
	void set_buffer_data(BufferHandle handle, const void* data, u32 size);

	void set_texture_data(TextureHandle handle, const void* data, u32 size);

	void set_uniforms(const SetUniformConfig& config);

	CommandsHandle record_persitent_commands(Function<void(Commands*)> fn);

	void submit(Function<void(Commands*)> fn);
	void submit(toki::Span<CommandsHandle> recorded_commands);
	void flush_queue(const SubmitOptions& options);

private:
	void frame_prepare();
	void frame_cleanup();
	Commands* get_commands();
	void present();

private:
	void* m_internalData{};
};

}  // namespace toki
