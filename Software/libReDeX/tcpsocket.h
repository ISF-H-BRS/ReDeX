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

#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------------------------- //

using namespace std::chrono_literals;

// ---------------------------------------------------------------------------------------------- //

struct sockaddr_in;

// ---------------------------------------------------------------------------------------------- //

class TcpAddress
{
public:
    enum class SpecialType { Any };

    using Bytes = std::array<uint8_t, 4>;
    using Error = std::runtime_error;

public:
    TcpAddress(const Bytes& bytes);
    TcpAddress(SpecialType);
    TcpAddress(const ::sockaddr_in& addr);

    auto bytes() const -> const Bytes&;
    auto operator[](size_t n) const -> uint8_t;
    auto toString() const -> std::string;

    static auto fromHostName(const std::string& name) -> TcpAddress;

private:
    std::array<uint8_t, 4> m_bytes = {};
};

// ---------------------------------------------------------------------------------------------- //

class TcpSocket
{
public:
    static constexpr std::chrono::milliseconds DefaultTimeout = 1000ms;

    using Error = std::runtime_error;

public:
    TcpSocket(const TcpAddress& address, uint16_t port,
              std::chrono::milliseconds timeout = DefaultTimeout);

    TcpSocket(int fd, const ::sockaddr_in& sockaddr);

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&& other);

    ~TcpSocket();

    auto operator=(const TcpSocket& other) = delete;
    auto operator=(TcpSocket&& other) -> TcpSocket&;

    auto address() const -> TcpAddress;
    auto port() const -> uint16_t;

    void sendData(std::span<const char> data);
    auto waitForDataWritten(std::chrono::milliseconds timeout = DefaultTimeout) -> bool;

    auto waitForDataAvailable(std::chrono::milliseconds timeout = DefaultTimeout) -> bool;
    auto readAllData() -> std::vector<char>;

private:
    enum class WaitType { Read, Write };
    auto wait(WaitType type, std::chrono::milliseconds timeout) -> bool;

    void move(TcpSocket&& other);

    void connect(const TcpAddress& address, uint16_t port, std::chrono::milliseconds timeout);
    void close();

    void setNonBlocking(bool nonblock);

    void throwSystemError(std::string error, int code = 0);

private:
    struct Private;
    std::unique_ptr<Private> d;

    TcpAddress m_address = TcpAddress::SpecialType::Any;
    uint16_t m_port = 0;
};

// ---------------------------------------------------------------------------------------------- //
