#pragma once

#include <toki/core/core.h>
#include <toki/renderer/renderer_allocators.h>

namespace toki::renderer {

using TempString = toki::BasicString<char, RendererBumpAllocator>;

template <typename T>
using TempDynamicArray = toki::DynamicArray<T, RendererBumpAllocator>;

template <typename T>
using PersistentDynamicArray = toki::DynamicArray<T, RendererPersistentAllocator>;

template <typename T, int N>
using PersistentArena = toki::Arena<T, N, RendererPersistentAllocator>;

}  // namespace toki::renderer
