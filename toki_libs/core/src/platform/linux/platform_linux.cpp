#include "../platform.h"

#if defined(TK_PLATFORM_LINUX)

#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "../../core/assert.h"
#include "../../core/string.h"
#include "linux_platform.h"

extern char** environ;

namespace toki {

void exit(i32 error) {
    TK_ASSERT_PLATFORM_ERROR(syscall(SYS_exit, error), "Error exiting program");
}

void debug_break() {
    signal(SIGTRAP, SIG_DFL);
}
extern char** environ;

const char* getenv(const char* var) {
    u32 length = strlen(var);
    char** env = environ;

    while (*env) {
        if (toki::strcmp(var, *env, length) && (*env)[length] == '=') {
            return &(*env)[length + 1];
        }
        env++;
    }

    return nullptr;
}

u64 time_microseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

u64 time_milliseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec / 1000ULL;
}

}  // namespace toki

#endif
