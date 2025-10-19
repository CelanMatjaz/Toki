#include "toki/core/utils/file.h"

#include <toki/core/common/assert.h>
#include <toki/core/platform/syscalls.h>

namespace toki {

File::File(StringView path, FileMode mode, u32 flags): File(Path(path), mode, flags) {}

File::File(const Path& path, FileMode mode, u32 flags) {
	open(path, mode, flags);
}

File::~File() {
	if (m_handle.valid()) {
		close();
	}
}

void File::seek(i64 pos, FileCursorStart start) {
	set_file_pointer(m_handle, pos, start).assert_ok();
}

u64 File::tell() const {
	return get_file_pointer(m_handle).assert_ok_and_value();
}

u64 File::write(const void* data, u64 size) {
	return toki::write(m_handle, data, size).assert_ok_and_value();
}

u64 File::read(void* data, u64 size) {
	return toki::read(m_handle, data, size).assert_ok_and_value();
}

void File::open(const Path& path, FileMode mode, u32 flags) {
	m_handle = toki::open(path.c_str(), mode, flags);
}

void File::close() {
	TK_ASSERT(m_handle.valid());
	toki::close(m_handle);
}

}  // namespace toki
