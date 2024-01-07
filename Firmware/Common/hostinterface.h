// ============================================================================================== //
//                                                                                                //
//  This file is part of the ISF ReDeX project.                                                   //
//                                                                                                //
//  Author:                                                                                       //
//  Marcel Hasler <mahasler@gmail.com>                                                            //
//                                                                                                //
//  Copyright (c) 2021 - 2023                                                                     //
//  Bonn-Rhein-Sieg University of Applied Sciences                                                //
//                                                                                                //
//  This program is free software: you can redistribute it and/or modify it under the terms       //
//  of the GNU General Public License as published by the Free Software Foundation, either        //
//  version 3 of the License, or (at your option) any later version.                              //
//                                                                                                //
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     //
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     //
//  See the GNU General Public License for more details.                                          //
//                                                                                                //
//  You should have received a copy of the GNU General Public License along with this program.    //
//  If not, see <https://www.gnu.org/licenses/>.                                                  //
//                                                                                                //
// ============================================================================================== //

#pragma once

#include "defaultstring.h"

#include <array>
#include <chrono>
#include <span>

using namespace std::chrono_literals;

class HostInterface
{
public:
    static constexpr const char* LineTerminator = "\r\n";
    static constexpr size_t LineTerminatorSize = 2;

    static constexpr size_t TransmitBufferSize = 2048;
    static constexpr size_t MaximumStringSpanSize = TransmitBufferSize / String::Capacity;

    static constexpr size_t ReceiveBufferSize = 256;

    static constexpr std::chrono::microseconds DefaultTimeout = 50ms;

    class Owner
    {
        friend class HostInterface;
        virtual void onHostDataReceived(const String& data) = 0;
        virtual void onHostDataOverflow() = 0;
    };

public:
    HostInterface(Owner* owner);
    ~HostInterface();

    void update();

    auto sendData(const String& data, std::chrono::microseconds timeout = DefaultTimeout) -> bool;

    auto sendData(std::span<const String> data,
                  std::chrono::microseconds timeout = DefaultTimeout) -> bool;

private:
    auto sendData(const uint8_t* data, uint32_t size, std::chrono::microseconds timeout) -> bool;
    void processData(const uint8_t* data, uint32_t size);

    static void cdcReceiveCallback(uint8_t* buffer, uint32_t size);
    static void cdcTxCompleteCallback();

private:
    Owner* m_owner;
    String m_currentData;

    std::array<uint8_t, TransmitBufferSize> m_txBuffer = {};

    std::array<uint8_t, ReceiveBufferSize> m_rxBuffer = {};
    volatile size_t m_rxBufferSize = 0;

    volatile bool m_rxBufferOverflow = false;
    volatile bool m_txComplete = true;

    static HostInterface* s_instance;
};
