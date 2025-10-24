#pragma once

#include <toki/core/core.h>
#include <toki/runtime/render/types.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

struct ObjData {
	DynamicArray<Vertex> vertex_data;
	DynamicArray<u32> index_data;
};

ObjData load_obj(const Path& path);

}  // namespace toki
