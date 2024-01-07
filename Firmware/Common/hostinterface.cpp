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

#include "assert.h"
#include "delay.h"
#include "hostinterface.h"

#include "usbd_cdc_if.h"

// ---------------------------------------------------------------------------------------------- //

HostInterface* HostInterface::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

HostInterface::HostInterface(Owner* owner)
    : m_owner(owner)
{
    ASSERT(m_owner != nullptr);

    s_instance = this;

    CDC_RegisterReceiveCallback(&HostInterface::cdcReceiveCallback);
    CDC_RegisterTxCompleteCallback(&HostInterface::cdcTxCompleteCallback);
}

// ---------------------------------------------------------------------------------------------- //

HostInterface::~HostInterface()
{
    CDC_RegisterReceiveCallback(nullptr);
    CDC_RegisterTxCompleteCallback(nullptr);

    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void HostInterface::update()
{
    static std::array<uint8_t, ReceiveBufferSize> workBuffer = {};

    while (m_rxBufferSize > 0)
    {
        HAL_NVIC_DisableIRQ(USB_IRQn);

        std::copy_n(m_rxBuffer.begin(), m_rxBufferSize, workBuffer.begin());

        size_t rxBufferSize = m_rxBufferSize;
        m_rxBufferSize = 0;

        HAL_NVIC_EnableIRQ(USB_IRQn);

        for (size_t i = 0; i < rxBufferSize; ++i)
        {
            m_currentData += workBuffer[i];

            if (m_currentData.endsWith(LineTerminator))
            {
                m_currentData.trim(LineTerminatorSize);
                m_owner->onHostDataReceived(m_currentData);
                m_currentData.clear();
            }
        }

        if (m_rxBufferOverflow)
        {
            m_rxBufferOverflow = false;
            m_owner->onHostDataOverflow();
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

auto HostInterface::sendData(const String& data, std::chrono::microseconds timeout) -> bool
{
    static const std::array<uint8_t, 2> terminator = { '\r', '\n' };

    std::copy(data.cbegin(), data.cend(), m_txBuffer.begin());
    std::copy(terminator.cbegin(), terminator.cend(), m_txBuffer.begin() + data.size());

    return sendData(m_txBuffer.data(), data.size() + terminator.size(), timeout);
}

// ---------------------------------------------------------------------------------------------- //

auto HostInterface::sendData(std::span<const String> data,
                             std::chrono::microseconds timeout) -> bool
{
    static const std::array<uint8_t, 2> terminator = { '\r', '\n' };

    size_t size = 0;

    for (const auto& s : data)
    {
        std::copy(s.cbegin(), s.cend(), m_txBuffer.begin() + size);
        std::copy(terminator.cbegin(), terminator.cend(), m_txBuffer.begin() + size + s.size());

        size += s.size() + terminator.size();
    }

    return sendData(m_txBuffer.data(), size, timeout);
}

// ---------------------------------------------------------------------------------------------- //

auto HostInterface::sendData(const uint8_t* data, uint32_t size,
                             std::chrono::microseconds timeout) -> bool
{
    m_txComplete = false;
    CDC_Transmit(const_cast<uint8_t*>(data), size);

    return Delay::wait(timeout, [this]{ return m_txComplete; });
}

// ---------------------------------------------------------------------------------------------- //

void HostInterface::processData(const uint8_t* data, uint32_t size)
{
    if (m_rxBufferSize + size > ReceiveBufferSize)
        m_rxBufferOverflow = true;
    else
    {
        std::copy_n(data, size, m_rxBuffer.begin() + m_rxBufferSize);
        m_rxBufferSize += size;
    }
}

// ---------------------------------------------------------------------------------------------- //

void HostInterface::cdcReceiveCallback(uint8_t* buffer, uint32_t size)
{
    ASSERT(s_instance != nullptr);
    s_instance->processData(buffer, size);
}

// ---------------------------------------------------------------------------------------------- //

void HostInterface::cdcTxCompleteCallback()
{
    ASSERT(s_instance != nullptr);
    s_instance->m_txComplete = true;
}

// ---------------------------------------------------------------------------------------------- //
