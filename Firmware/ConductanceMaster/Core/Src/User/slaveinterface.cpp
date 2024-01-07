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
#include "slaveinterface.h"

// ---------------------------------------------------------------------------------------------- //

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    auto instance = SlaveInterface::s_instance;

    ASSERT(instance != nullptr);
    ASSERT(htim == instance->m_timerHandle);

    instance->onTimerPeriodElapsed();
}

// ---------------------------------------------------------------------------------------------- //

SlaveInterface* SlaveInterface::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

SlaveInterface::SlaveInterface(Owner* owner)
    : m_owner(owner)
{
    ASSERT(owner != nullptr);
    s_instance = this;
}

// ---------------------------------------------------------------------------------------------- //

SlaveInterface::~SlaveInterface()
{
    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

auto SlaveInterface::getInputConnected(size_t input) const -> bool
{
    ASSERT(input < InputCount);
    return HAL_GPIO_ReadPin(m_conPorts[input], m_conPins[input]) == GPIO_PIN_RESET;
}

// ---------------------------------------------------------------------------------------------- //

auto SlaveInterface::getInputMask() const -> uint8_t
{
    uint8_t mask = 0x00;

    for (size_t i = 0; i < InputCount; ++i)
        mask |= (getInputConnected(i) << i);

    return mask;
}

// ---------------------------------------------------------------------------------------------- //

void SlaveInterface::startCapture()
{
    HAL_TIM_Base_Start_IT(m_timerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SlaveInterface::stopCapture()
{
    HAL_TIM_Base_Stop_IT(m_timerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SlaveInterface::setCommand(size_t input, uint32_t command)
{
    ASSERT(input < InputCount);
    m_nextCommands[input] = command;
}

// ---------------------------------------------------------------------------------------------- //

void SlaveInterface::onTimerPeriodElapsed()
{
    std::array<bool, InputCount> connected = { false, false };
    std::array<bool, InputCount> dataReady = { false, false };

    using SampleArray = std::array<uint16_t, 2>;
    std::array<SampleArray, InputCount> samples = {{ { 0, 0 }, { 0, 0 } }};

    // Check connection and select slave
    for (size_t i = 0; i < InputCount; ++i)
    {
        connected[i] = HAL_GPIO_ReadPin(m_conPorts[i], m_conPins[i]) == GPIO_PIN_RESET;

        if (connected[i])
            HAL_GPIO_WritePin(m_csPorts[i], m_csPins[i], GPIO_PIN_RESET);
    }

    // Start SPI units && clear spurious data
    for (size_t i = 0; i < InputCount; ++i)
    {
        __HAL_SPI_ENABLE(m_spiHandles[i]);

        while (__HAL_SPI_GET_FLAG(m_spiHandles[i], SPI_FLAG_RXNE) == SET)
            m_spiHandles[i]->Instance->DR;
    }

    // Wait for data to become ready
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (!connected[i])
            continue;

        for (unsigned int timeout = 0; timeout < 100; ++timeout)
        {
            if (HAL_GPIO_ReadPin(m_drdyPorts[i], m_drdyPins[i]) == GPIO_PIN_RESET)
            {
                dataReady[i] = true;
                break;
            }
        }
    }

    // Transmit low word of command
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (dataReady[i])
            m_spiHandles[i]->Instance->DR = static_cast<uint16_t>(m_nextCommands[i]);
    }

    // Read first sample
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (dataReady[i])
        {
            while (__HAL_SPI_GET_FLAG(m_spiHandles[i], SPI_FLAG_RXNE) == RESET)
                continue;

            samples[i][0] = m_spiHandles[i]->Instance->DR;
        }
    }

    // Transmit high word of command
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (dataReady[i])
            m_spiHandles[i]->Instance->DR = static_cast<uint16_t>(m_nextCommands[i] >> 16);
    }

    // Read second sample
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (dataReady[i])
        {
            while (__HAL_SPI_GET_FLAG(m_spiHandles[i], SPI_FLAG_RXNE) == RESET)
                continue;

            samples[i][1] = m_spiHandles[i]->Instance->DR;
        }
    }

    // Release slaves, shutdown SPI units & store data
    for (size_t i = 0; i < InputCount; ++i)
    {
        if (connected[i])
        {
            HAL_GPIO_WritePin(m_csPorts[i], m_csPins[i], GPIO_PIN_SET);
            __HAL_SPI_DISABLE(m_spiHandles[i]);
        }

        m_data[m_queuePosition][i*DataChannelCount*DataSamplesCount
                                    + 0*DataSamplesCount + m_samplePosition] = samples[i][0];

        m_data[m_queuePosition][i*DataChannelCount*DataSamplesCount
                                    + 1*DataSamplesCount + m_samplePosition] = samples[i][1];
        m_nextCommands[i] = 0;
    }

    // Notify owner
    if (++m_samplePosition >= DataSamplesCount)
    {
        auto ptr = reinterpret_cast<uint8_t*>(m_data[m_queuePosition].data());

        m_owner->onSlaveBufferFull({ ptr, DataBufferSize });
        m_samplePosition = 0;

        if (++m_queuePosition >= DataQueueLength)
            m_queuePosition = 0;
    }
}

// ---------------------------------------------------------------------------------------------- //
