#pragma once

#include "core/base.h"

namespace toki {

namespace platform {

// NOTE: Preparation for wide strings
using PATH = const wchar_t*;

static_assert(sizeof(wchar_t) == 2);

enum FileOpenFlags {
	// Open file for reading
	FileOpen_Read = BIT(0),

	// Open file for writing
	FileOpen_Write = BIT(1),

	// Open file for reading and writing
	FileOpen_ReadWrite = FileOpen_Read | FileOpen_Write,

	// Open existing file
	//
	// File needs to exist, or call will fail
	// and return a file with an invalid handle
	FileOpen_OpenExisting = BIT(2),

	// Create new file or overwrite existing one
	FileOpen_CreateNew = BIT(3),
};

enum FileCreateAttributeFlags {
	// Create file without any specific flags - default value
	FileCreateAttribute_Normal = 0,

	// Create hidden file
	FileCreateAttribute_Hidden = BIT(0),

	// Create read-only file
	FileCreateAttribute_ReadOnly = BIT(1),
};

// Internal file type
struct File {
	void* native_handle;
};

// Opens file with provided parameters
[[nodiscard]] File file_open(PATH path, u32 open_flags, u32 create_attribute_flags = FileCreateAttribute_Normal);

// Closes provided file
//
// Expects a file with a valid handle
void file_close(File* file);

// Deletes provided file
//
// Expects a file with a valid handle
void file_delete(PATH path);

// Checks if file exists
bool file_exists(PATH path);

// Creates new directory recursively
//
// Ensures that the provided path exists after call
void directory_create(PATH path);

// Normalized path string for specific platform
void normalize_path(const wchar_t* path, wchar_t* path_out, u32 path_out_size);

}  // namespace platform

}  // namespace toki
