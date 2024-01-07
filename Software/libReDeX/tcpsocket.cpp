/**************************************************************************************************
 *                                                                                                *
 *  This file is part of the ISF Utilities collection.                                            *
 *                                                                                                *
 *  Author:                                                                                       *
 *  Marcel Hasler <marcel.hasler@h-brs.de>                                                        *
 *                                                                                                *
 *  Copyright (c) 2022                                                                            *
 *  Bonn-Rhein-Sieg University of Applied Sciences                                                *
 *                                                                                                *
 *  Redistribution and use in source and binary forms, with or without modification,              *
 *  are permitted provided that the following conditions are met:                                 *
 *                                                                                                *
 *  1. Redistributions of source code must retain the above copyright notice,                     *
 *     this list of conditions and the following disclaimer.                                      *
 *                                                                                                *
 *  2. Redistributions in binary form must reproduce the above copyright notice,                  *
 *     this list of conditions and the following disclaimer in the documentation                  *
 *     and/or other materials provided with the distribution.                                     *
 *                                                                                                *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"                   *
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED             *
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.            *
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,              *
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT            *
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR            *
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,             *
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)            *
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE                    *
 *  POSSIBILITY OF SUCH DAMAGE.                                                                   *
 *                                                                                                *
 **************************************************************************************************/

#include "tcpsocket.h"

#if defined(__linux__)
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h> // for getaddrinfo()
#include <iostream>
#endif

#include <algorithm>
#include <cstring>

// ---------------------------------------------------------------------------------------------- //

#if defined(_WIN32)
static class WinSockInit
{
public:
    WinSockInit()
    {
        ::WSADATA wsadata;
        if (::WSAStartup(MAKEWORD(2,0), &wsadata) != 0)
        {
            std::cerr << "Unable to init WinSock." << std::endl;
            exit(-1);
        }
    }

    ~WinSockInit()
    {
        ::WSACleanup();
    }
} init;
#endif

// ---------------------------------------------------------------------------------------------- //

TcpAddress::TcpAddress(const Bytes& bytes)
    : m_bytes(bytes) {}

// ---------------------------------------------------------------------------------------------- //

TcpAddress::TcpAddress(SpecialType) {}

// ---------------------------------------------------------------------------------------------- //

TcpAddress::TcpAddress(const ::sockaddr_in& addr)
{
    uint32_t address = ::ntohl(addr.sin_addr.s_addr);

    m_bytes = {
        static_cast<uint8_t>(address>>24),
        static_cast<uint8_t>(address>>16),
        static_cast<uint8_t>(address>> 8),
        static_cast<uint8_t>(address>> 0)
    };
}

// ---------------------------------------------------------------------------------------------- //

auto TcpAddress::bytes() const -> const Bytes&
{
    return m_bytes;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpAddress::operator[](size_t n) const -> uint8_t
{
    return n < m_bytes.size() ? m_bytes[n] : 0;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpAddress::toString() const -> std::string
{
    return std::to_string(m_bytes[0]) + "." +
           std::to_string(m_bytes[1]) + "." +
           std::to_string(m_bytes[2]) + "." +
           std::to_string(m_bytes[3]);
}

// ---------------------------------------------------------------------------------------------- //

auto TcpAddress::fromHostName(const std::string& name) -> TcpAddress
{
    ::addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    ::addrinfo* info = nullptr;
    int result = ::getaddrinfo(name.c_str(), nullptr, &hints, &info);

    if (result != 0)
        throw Error("Unable to resolve host name " + name + ".");

    auto sockaddr = reinterpret_cast<::sockaddr_in*>(info->ai_addr);

    TcpAddress address(*sockaddr);
    freeaddrinfo(info);

    return address;
}

// ============================================================================================== //

struct TcpSocket::Private
{
#if defined(__linux__)
    int fd = -1;
#elif defined(_WIN32)
    SOCKET fd = INVALID_SOCKET;
#endif
};

// ---------------------------------------------------------------------------------------------- //

TcpSocket::TcpSocket(const TcpAddress& address, uint16_t port, std::chrono::milliseconds timeout)
    : d(std::make_unique<Private>()),
      m_address(address),
      m_port(port)
{
    static const auto socketValid = [](auto socket)
    {
#if defined(__linux__)
        return socket >= 0;
#elif defined(_WIN32)
        return socket != INVALID_SOCKET;
#endif
    };

    d->fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!socketValid(d->fd))
        throwSystemError("Unable to create TCP socket:");

    try {
        connect(address, port, timeout);
    }
    catch (...) {
        close();
        throw;
    }
}

// ---------------------------------------------------------------------------------------------- //

TcpSocket::TcpSocket(int fd, const ::sockaddr_in& sockaddr)
    : d(std::make_unique<Private>()),
      m_address(sockaddr),
      m_port(sockaddr.sin_port)
{
    d->fd = fd;
}

// ---------------------------------------------------------------------------------------------- //

TcpSocket::TcpSocket(TcpSocket&& other)
    : d(std::make_unique<Private>())
{
    move(std::move(other));
}

// ---------------------------------------------------------------------------------------------- //

TcpSocket::~TcpSocket()
{
    close();
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::operator=(TcpSocket&& other) -> TcpSocket&
{
    close();
    move(std::move(other));

    return *this;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::address() const -> TcpAddress
{
    return m_address;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::port() const -> uint16_t
{
    return m_port;
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::sendData(std::span<const char> data)
{
#if defined(_WIN32)
    constexpr int MSG_NOSIGNAL = 0; // Unix-specific define
#endif

    int64_t sent = ::send(d->fd, data.data(), data.size(), MSG_NOSIGNAL);

    if (sent < 0)
        throwSystemError("Writing to TCP socket failed:");
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::waitForDataWritten(std::chrono::milliseconds timeout) -> bool
{
    return wait(WaitType::Write, timeout);
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::waitForDataAvailable(std::chrono::milliseconds timeout) -> bool
{
    return wait(WaitType::Read, timeout);
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::readAllData() -> std::vector<char>
{
#if defined(__linux__)
    int size = 0;
    ::ioctl(d->fd, FIONREAD, &size);
#elif defined(_WIN32)
    u_long size = 0;
    ::ioctlsocket(d->fd, FIONREAD, &size);
#endif

    std::vector<char> data(size);

    int64_t read = ::recv(d->fd, data.data(), data.size(), 0);

    if (read < 0 || static_cast<size_t>(read) != data.size())
        throwSystemError("Reading from TCP socket failed:");

    return data;
}

// ---------------------------------------------------------------------------------------------- //

auto TcpSocket::wait(WaitType type, std::chrono::milliseconds timeout) -> bool
{
    ::pollfd fd = {};
    fd.fd = d->fd;

    if (type == WaitType::Read)
        fd.events = POLLIN;
    else
        fd.events = POLLOUT;

    const auto end = std::chrono::steady_clock::now() + timeout;

    int result = 0;

    do {
        const auto remaining = end - std::chrono::steady_clock::now();
        timeout = std::chrono::duration_cast<std::chrono::milliseconds>(remaining);

#if defined(__linux__)
        result = ::poll(&fd, 1, static_cast<int>(timeout.count()));
    } while (result < 0 && errno == EINTR);
#elif defined(_WIN32)
        result = ::WSAPoll(&fd, 1, static_cast<int>(timeout.count()));
    } while (result < 0 && ::WSAGetLastError() == WSAEINTR);
#endif
    return result > 0;
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::move(TcpSocket&& other)
{
    d->fd = other.d->fd;

#if defined(__linux__)
    other.d->fd = -1;
#elif defined(_WIN32)
    other.d->fd = INVALID_SOCKET;
#endif

    m_address = other.m_address;
    other.m_address = TcpAddress::SpecialType::Any;

    m_port = other.m_port;
    other.m_port = 0;
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::connect(const TcpAddress& address, uint16_t port, std::chrono::milliseconds timeout)
{
    setNonBlocking(true);

    ::sockaddr_in sockaddr = {};
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = ::htons(port);
    sockaddr.sin_addr.s_addr = ::htonl((address[0]<<24) | (address[1]<<16) |
                                       (address[2]<< 8) | (address[3]));

    // Non-blocking connect doesn't return success
    int result = ::connect(d->fd, reinterpret_cast<::sockaddr*>(&sockaddr), sizeof(sockaddr));

    if (result == 0)
        throwSystemError("Unable to initiate TCP connection:");

    // Wait for socket to become writable
    if (!wait(WaitType::Write, timeout))
        throw Error("Connection timed out.");

    // Get status
    socklen_t socklen = sizeof(result);

    if (::getsockopt(d->fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&result), &socklen) < 0)
        throwSystemError("Unable to retrieve TCP socket error status:");

    if (result != 0)
        throwSystemError("Unable to connect to host:", result);

    setNonBlocking(false);
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::close()
{
    if (d->fd >= 0)
    {
#if defined(__linux__)
        ::shutdown(d->fd, SHUT_RDWR);
        ::close(d->fd);
#elif defined(_WIN32)
        ::shutdown(d->fd, SD_BOTH);
        ::closesocket(d->fd);
#endif
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::setNonBlocking(bool nonblock)
{
#if defined(__linux__)
    int flags = ::fcntl(d->fd, F_GETFL, 0);

    if (flags < 0)
        throwSystemError("Unable to retrieve socket status flags:");

    if (nonblock)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (::fcntl(d->fd, F_SETFL, flags) < 0)
        throwSystemError("Unable to set socket status flags:");
#elif defined(_WIN32)
    unsigned long mode = nonblock ? 1 : 0;

    if (::ioctlsocket(d->fd, FIONBIO, &mode) < 0)
        throwSystemError("Unable to set socket blocking mode:");
#endif
}

// ---------------------------------------------------------------------------------------------- //

void TcpSocket::throwSystemError(std::string error, int code)
{
#if defined(__linux__)
    if (code == 0)
        code = errno;

    error = error + " " + std::string(::strerror(code)) + ".";
#elif defined(_WIN32)
    if (code == 0)
        code = ::WSAGetLastError();

    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;
    LPSTR buffer = nullptr;

    ::FormatMessageA(flags, nullptr, code, 0, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

    error = error + " " + std::string(buffer) + ".";

    ::LocalFree(buffer);
#endif

    throw Error(error);
};

// ---------------------------------------------------------------------------------------------- //
