#pragma once

#include "../core/common.h"
#include "../core/macros.h"
#include "file.h"

namespace toki {

class Stream {
public:
    using OffsetType = u32;

    Stream(): _file(), _offset(0) {}
    Stream(const char* path, FileOpenFlags flags = FILE_OPEN_READ): _file(path, flags), _offset(0) {}
    ~Stream() {
        _file.close();
    }

    DELETE_COPY(Stream);

    Stream(Stream&& other) {
        swap(move(other));
    }

    Stream& operator=(Stream&& other) {
        if (this != &other) {
            swap(move(other));
        }
        return *this;
    }

    void seek(OffsetType offset);
    OffsetType tell();

private:
    void swap(Stream&& other) {
        _file = move(other._file);
        other._file = {};
        _offset = other._offset;
        other._offset = {};
    }

    File _file{};
    OffsetType _offset{};
};

}  // namespace toki
