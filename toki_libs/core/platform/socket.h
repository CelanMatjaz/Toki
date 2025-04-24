#pragma once

#include "../core/common.h"
#include "../utils/macros.h"
#include "platform.h"

namespace toki {

class Socket {
public:
    enum class ConnectionType {
        LOCAL,
    };

    enum class State {
        UNINITIALIZED = 0,
        CONNECTED
    };

    struct ConnectOptions {
        char* address;
    };

public:
    Socket();
    Socket(ConnectionType connection_type, const ConnectOptions& options);

    ~Socket();

    DELETE_COPY(Socket);

    Socket(Socket&& other) {
        swap(move(other));
    }

    Socket& operator=(Socket&& other) {
        if (this != &other) {
            swap(move(other));
        }

        return *this;
    }

    void connect(ConnectionType connection_type, const ConnectOptions& options);
    void close();

    u64 receive_blocking(u64 n, void* data);
    u64 send(u64 n, const void* data);

    NativeHandle handle() const {
        return _handle;
    }

private:
    void swap(Socket&& other) {
        _handle = other._handle;
        other._handle = {};
        _state = other._state;
        other._state = {};
    }

    NativeHandle _handle{};
    State _state{};
};

}  // namespace toki
