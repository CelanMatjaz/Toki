#pragma once

#include "core/core.h"
#include "vector"
#include "glm/glm.hpp"

namespace Toki {

	enum class RenderTarget {
		None, Swapchain, Texture
	};

	enum class Format {
		RGBA8, R8, Depth
	};

	enum class Samples {
		Sample1, Sample2, Sample4, Sample8, Sample16, Sample32, Sample64
	};

	enum class AttachmentStoreOp {
		DontCare, Store
	};

	enum class AttachmentLoadOp {
		DontCare, Load, Clear
	};

	struct Attachment {
		Format format;
		Samples samples = Samples::Sample1;
		AttachmentLoadOp loadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
		RenderTarget target = RenderTarget::Texture;
	};

	struct FramebufferConfig {
		uint32_t width, height;
		std::vector<Attachment> colorAttachments;
		Ref<Attachment> depthAttachment;
		RenderTarget target = RenderTarget::None;
		glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	};

	class Framebuffer {
	public:
		static Ref<Framebuffer> create(const FramebufferConfig& config);

		Framebuffer(const FramebufferConfig& config);
		virtual ~Framebuffer() = default;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		void setClearColor(glm::vec4 color = { 0.1f, 0.1f, 0.1f, 0.1f }) {
			config.clearColor = color;
		}

	protected:
		FramebufferConfig config;
	};

}