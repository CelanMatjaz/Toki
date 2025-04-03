#pragma once

#include "../core/base.h"
#include "../core/macros.h"
#include "platform.h"

namespace toki {

void file_delete(PATH_TYPE path);

b8 file_exists(PATH_TYPE path);

b8 directory_exists(PATH_TYPE path);

class Stream {
public:
    using STREAM_OFFSET_TYPE = i64;

    enum STREAM_FLAGS : u32 {
        INPUT = BIT(0),
        OUTPUT = BIT(1),
        ATE = BIT(2),
        TRUNC = BIT(3),
    };

    Stream() = delete;
    Stream(PATH_TYPE path, u32 flags);
    ~Stream();

    DELETE_COPY(Stream);

    Stream(Stream&& other) {
        swap(MOVE(other));
    }

    Stream& operator=(Stream&& other) {
        if (this != &other) {
            swap(MOVE(other));
        }
        return *this;
    }

    void seek(STREAM_OFFSET_TYPE offset);
    STREAM_OFFSET_TYPE tell();
    void close();

    void swap(Stream&& other) {
        m_Offset = other.m_Offset;
        m_NativeHandle = other.m_NativeHandle;
        other.m_NativeHandle.i64 = 0;
    }

private:
    STREAM_OFFSET_TYPE m_Offset{ 0 };
    NativeWindowHandle m_NativeHandle{};
};

}  // namespace toki
