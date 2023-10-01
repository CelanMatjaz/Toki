#pragma once

#include "core/core.h"
#include "vector"
#include "glm/glm.hpp"
#include "texture.h"
#include "render_pass.h"

namespace Toki {

	struct FramebufferConfig {
		uint32_t width, height;
		RenderTarget target = RenderTarget::Swapchain;
		glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		Ref<RenderPass> renderPass;
	};

	class Framebuffer {
	public:
		static Ref<Framebuffer> create(const FramebufferConfig& config);

		Framebuffer(const FramebufferConfig& config);
		virtual ~Framebuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;
		virtual void resize(uint32_t width, uint32_t height, uint32_t layers = 1) = 0;
		virtual void nextSubpass() = 0;
		virtual float readPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, uint32_t z) = 0;
		virtual Ref<Texture> getAttachment(uint32_t attachmentIndex) = 0;

		void setClearColor(glm::vec4 color = { 0.1f, 0.1f, 0.1f, 0.1f }) {
			config.clearColor = color;
		}

	protected:
		FramebufferConfig config;
	};

}