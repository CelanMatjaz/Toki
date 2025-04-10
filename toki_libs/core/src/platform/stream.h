#pragma once

#include "../core/common.h"
#include "../utils/macros.h"
#include "file.h"

namespace toki {

class Stream : public File {
public:
    using OffsetType = u32;

    Stream(): File() {}
    Stream(const char* path, u32 flags = FILE_OPEN_READ): File(path, flags), _offset(0) {}
    ~Stream() {
        File::close();
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
        File::swap(move(reinterpret_cast<File&>(other)));
        _offset = other._offset;
        other._offset = {};
    }

    OffsetType _offset{};
};

}  // namespace toki
