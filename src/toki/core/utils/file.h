#pragma once

#include <toki/core/platform/defines.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/utils/path.h>

namespace toki {

class File {
public:
	File() = default;
	File(StringView path, FileMode mode, u32 flags = 0);
	File(const Path& path, FileMode mode, u32 flags = 0);
	~File();

	void seek(i64 pos, FileCursorStart start = FileCursorStart::CURRENT);
	u64 tell() const;

	u64 write(const void* data, u64 size);
	u64 read(void* data, u64 size);

	void open(const Path& path, FileMode mode, u32 flags = 0);
	void close();

	u64 read_line(char* data, u64 count, byte delim = '\n');

private:
	u64 m_cursor{};
	NativeHandle m_handle{};
};

}  // namespace toki
