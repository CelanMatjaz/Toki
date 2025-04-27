#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/un.h>

#include "../socket.h"
#include "core/assert.h"
#include "core/string.h"
#include "linux_platform.h"
#include "platform/platform.h"

namespace toki {

Socket::Socket(): _handle(), _state(State::UNINITIALIZED) {}

Socket::Socket(ConnectionType connection_type, const ConnectOptions& options): Socket() {
    connect(connection_type, options);
}

Socket::~Socket() {
    close();
}

void Socket::connect(ConnectionType connection_type, const ConnectOptions& options) {
    TK_ASSERT(_state == State::UNINITIALIZED, "Cannot connect a socket that already has state set");
    _handle = toki::syscall(SYS_socket, AF_UNIX, SOCK_STREAM, 0);
    TK_ASSERT_PLATFORM_ERROR(_handle, "Could not create socket");

    switch (connection_type) {
        case ConnectionType::LOCAL: {
            struct sockaddr_un addr = { .sun_family = AF_UNIX, .sun_path = {} };
            toki::memcpy(options.address, addr.sun_path, toki::strlen(options.address));
            TK_ASSERT_PLATFORM_ERROR(
                toki::syscall(SYS_connect, _handle, (struct sockaddr*) &addr, sizeof(addr)),
                "Could not connected to socket");
        } break;

        default:
            TK_UNREACHABLE();
    }

    _state = State::CONNECTED;
}

void Socket::close() {
    if (_handle == NativeHandle::INVALID_HANDLE_ID) {
        return;
    }

    TK_ASSERT_PLATFORM_ERROR(toki::syscall(SYS_close, _handle), "Could not close socket");
    _state = State::UNINITIALIZED;
}

u64 Socket::receive_blocking(u64 n, void* data) {
    TK_ASSERT(_state != State::UNINITIALIZED, "Cannot receive message with non initialized socket");
    struct iovec io = { .iov_base = data, .iov_len = n };
    struct msghdr msg{};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    i64 read_byte_count = toki::syscall(SYS_recvmsg, _handle, &msg, 0);
    TK_ASSERT_PLATFORM_ERROR(read_byte_count, "Error waiting for new message");
    return read_byte_count;
}

u64 Socket::send(u64 n, const void* data) {
    TK_ASSERT(_state != State::UNINITIALIZED, "Cannot send message with non initialized socket");
    struct iovec io = { .iov_base = const_cast<void*>(data), .iov_len = n };
    struct msghdr msg{};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    i64 sent_byte_count = toki::syscall(SYS_sendmsg, _handle, &msg, 0);
    TK_ASSERT_PLATFORM_ERROR(sent_byte_count, "Error sending message");
    return sent_byte_count;
}

}  // namespace toki
