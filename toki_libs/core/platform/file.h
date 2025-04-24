#pragma once

#include "../core/common.h"
#include "../utils/macros.h"
#include "platform.h"

namespace toki {

enum FileOpenFlags : u32 {
    FILE_OPEN_READ = 0x0001,
    FILE_OPEN_WRITE = 0x0002,
    FILE_OPEN_APPEND = 0x0004,
    FILE_OPEN_TRUNC = 0x0008,
    FILE_OPEN_RDWR = FILE_OPEN_READ | FILE_OPEN_WRITE
};

class File {
public:
    File(): _handle(), _is_open(false), _flags() {}
    File(const char* path, u32 flags = FILE_OPEN_READ) {
        open(path, flags);
    }
    ~File() {
        close();
    }

    File(File&& other) {
        swap(move(other));
    }

    File& operator=(File&& other) {
        swap(move(other));
        return *this;
    }

    DELETE_COPY(File)

    void open(const char* path, u32 flags = FILE_OPEN_READ);
    void close();

    void write(u32 n, const void* data);
    void read(u32 n, void* data);

    NativeHandle handle() const {
        return _handle;
    }

    b8 is_open() const {
        return _is_open;
    }

protected:
    void swap(File&& other) {
        if (this != &other) {
            _handle = other._handle;
            other._handle = {};
            _is_open = other._is_open;
            other._is_open = {};
            _flags = other._flags;
            other._flags = {};
        }
    }

    NativeHandle _handle{};
    b32 _is_open{};
    u32 _flags{};
};

}  // namespace toki
