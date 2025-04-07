#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "../../core/assert.h"
#include "../stream.h"

namespace toki {

void Stream::seek(OffsetType offset) {
    _offset = syscall(SYS_lseek, _file.handle(), offset, SEEK_SET);
    TK_ASSERT_PLATFORM_ERROR(_offset, "Could not set file offset");
}

Stream::OffsetType Stream::tell() {
    _offset = syscall(SYS_lseek, _file.handle(), 0, SEEK_CUR);
    TK_ASSERT_PLATFORM_ERROR(_offset, "Could not get file offset");
    return _offset;
}

}  // namespace toki
