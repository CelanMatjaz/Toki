#include <fcntl.h>
#include <linux/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "../../core/assert.h"
#include "../filesystem.h"
#include "platform/linux/linux_platform.h"

namespace toki {

void file_delete(const char* path) {
    syscall(SYS_unlink, path);
}

b8 path_exists(const char* path) {
    i64 result = syscall(SYS_access, path, F_OK);
    TK_ASSERT_PLATFORM_ERROR(result, "Error checking if path exists");
    return result == 0;
}

b8 path_is_file(const char* path) {
    struct stat stat{};
    i64 result = syscall(SYS_newfstatat, AT_FDCWD, path, &stat, 0);
    TK_ASSERT_PLATFORM_ERROR(result, "Error getting stat for path");
    return S_ISREG(stat.st_mode);
}

b8 path_is_directory(const char* path) {
    struct stat stat{};
    i64 result = syscall(SYS_newfstatat, AT_FDCWD, path, &stat, 0);
    TK_ASSERT_PLATFORM_ERROR(result, "Error getting stat for path");
    return S_ISDIR(stat.st_mode);
}

}  // namespace toki
