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
#include "masterboard.h"

#include <algorithm>

// ---------------------------------------------------------------------------------------------- //

namespace ControlRequest {
    constexpr uint8_t GetInputMask = 0x01;
    constexpr uint8_t SetInputCommand = 0x02;
    constexpr uint8_t GetPowerValues = 0x03;
}

// ---------------------------------------------------------------------------------------------- //

MasterBoard::MasterBoard()
    : m_hostInterface(this),
      m_slaveInterface(this)
{
    // Reduces noise in supply voltage
    HAL_EnableCompensationCell();
}

// ---------------------------------------------------------------------------------------------- //

MasterBoard::~MasterBoard()
{
    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_SET);
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::exec()
{
    // Give slaves time to power up
    HAL_Delay(200);

    // Wait until full bus power is available
    while (!m_hostInterfaceReady)
        continue;

    // Enable status LED
    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_RESET);

    // Start sampling
    m_slaveInterface.startCapture();

    // Start main loop
    while (true)
    {
        m_powerMonitor.update();
        HAL_Delay(100);
    }
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::onInitComplete()
{
    m_hostInterfaceReady = true;
    m_bulkInComplete = true;
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::onControlOutReceived(uint8_t request, std::span<const uint8_t> data)
{
    if (request == ControlRequest::SetInputCommand)
    {
        struct Parameters
        {
            uint8_t input;
            uint32_t command;

        } __attribute__((__packed__));

        if (data.size() == sizeof(Parameters))
        {
            const auto parameters = reinterpret_cast<const Parameters*>(data.data());

            if (parameters->input < SlaveInterface::InputCount)
                m_slaveInterface.setCommand(parameters->input, parameters->command);
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::onControlInReceived(uint8_t request, std::span<uint8_t> data)
{
    static constexpr size_t InputMaskSize = sizeof(uint8_t);

    static constexpr size_t PowerValuesCount = 3;
    static constexpr size_t PowerValuesSize = PowerValuesCount * sizeof(uint16_t);

    if (request == ControlRequest::GetInputMask && data.size() == InputMaskSize)
    {
        uint8_t mask = m_slaveInterface.getInputMask();
        data[0] = mask;
    }
    else if (request == ControlRequest::GetPowerValues && data.size() == PowerValuesSize)
    {
        const std::array<uint16_t, PowerValuesCount> values = {
            static_cast<uint16_t>(m_powerMonitor.getVoltage() * 1000.0),
            static_cast<uint16_t>(m_powerMonitor.getCurrent() * 1000.0),
            static_cast<uint16_t>(m_powerMonitor.getTemperature() * 100.0),
        };

        std::copy_n(reinterpret_cast<const uint8_t*>(values.data()), data.size(), data.data());
    }
    else // Invalid request, or invalid length
        std::fill(data.begin(), data.end(), 0);
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::sendNextBulkData()
{
    ASSERT(m_bulkInComplete && m_nextData != nullptr);

    m_hostInterface.sendBulkInData({ m_nextData, SlaveInterface::DataBufferSize });
    m_bulkInComplete = false;
    m_nextData = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::onBulkInComplete()
{
    m_bulkInComplete = true;

    if (m_nextData != nullptr)
        sendNextBulkData();
}

// ---------------------------------------------------------------------------------------------- //

void MasterBoard::onSlaveBufferFull(std::span<const uint8_t> data)
{
    m_nextData = data.data();

    if (m_bulkInComplete)
        sendNextBulkData();
}

// ---------------------------------------------------------------------------------------------- //
