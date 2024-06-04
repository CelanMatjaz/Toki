#pragma once

#include <array>

#include "renderer/buffer.h"
#include "renderer/command_pool.h"
#include "renderer/framebuffer.h"
#include "renderer/pipeline.h"
#include "renderer/render_pass.h"
#include "renderer/render_thread.h"
#include "renderer/sampler.h"
#include "renderer/swapchain.h"
#include "renderer/texture.h"
#include "renderer/vulkan_types.h"

namespace Toki {

#ifndef DEFAULT_GEOMETRY_VERTEX_BUFFER_SIZE
#define DEFAULT_GEOMETRY_VERTEX_BUFFER_SIZE 16 * 1024 * 1024
#endif

#ifndef DEFAULT_GEOMETRY_INDEX_BUFFER_SIZE
#define DEFAULT_GEOMETRY_INDEX_BUFFER_SIZE 16 * 1024 * 1024
#endif

#ifndef EXTRA_COMMAND_POOL_COUNT
#define EXTRA_COMMAND_POOL_COUNT 2
#endif

#ifndef MAX_DESCRIPTOR_SETS
#define MAX_DESCRIPTOR_SETS 100
#endif

// Maps of handles to objects
#pragma region ObjectHandles

inline std::unordered_map<AttachmentFormatHash, Ref<RenderPass>, AttachmentFormatHash> s_renderPassMap;

inline std::unordered_map<Handle, Ref<Swapchain>, Handle> s_swapchainMap;
inline std::unordered_map<Handle, Ref<Buffer>, Handle> s_bufferMap;
inline std::unordered_map<Handle, Ref<Texture>, Handle> s_textureMap;
inline std::unordered_map<Handle, Ref<Framebuffer>, Handle> s_framebufferMap;
inline std::unordered_map<Handle, Ref<Pipeline>, Handle> s_pipelineMap;
inline std::unordered_map<Handle, Ref<Sampler>, Handle> s_samplerMap;

inline Ref<Pipeline> s_currentlyBoundPipeline;
inline Ref<Framebuffer> s_currentlyBoundFramebuffer;

#pragma endregion ObjectHandles

// Uploaded geometry data
#pragma region GeometryData

inline std::vector<Scope<Buffer>> s_geometryVertexBuffers;
inline uint32_t s_currentGeometryVertexBufferIndex = 0;
inline uint32_t s_geometryVertexBufferOffset = 0;

inline std::vector<Scope<Buffer>> s_geometryIndexBuffers;
inline uint32_t s_currentGeometryIndexBufferIndex = 0;
inline uint32_t s_geometryIndexBufferOffset = 0;

#pragma endregion GeometryData

// Rendering state
#pragma region RenderingState

struct VulkanClearValues {
    VkClearColorValue colorClear{ { 0, 0, 0, 1 } };
    float depthClear = 0.0f;
    uint32_t stencilClear = 0;
} inline s_globalClearValues;

#pragma endregion RenderingState

// State needed for recording commands and presenting
#pragma region FrameState

inline uint32_t s_renderThreadCount = 3;
inline std::vector<std::array<CommandPool, MAX_FRAMES>> s_commandPools;
inline std::vector<CommandPool> s_extraCommandPools;
inline VkDescriptorPool s_descriptorPool = VK_NULL_HANDLE;

inline uint32_t s_currentFrameIndex = 0;
inline FrameContext s_frameContext[MAX_FRAMES];

#pragma endregion FrameState

}  // namespace Toki
