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
#include "signalgenerator.h"
#include "signalreader.h"

// ---------------------------------------------------------------------------------------------- //

void SamplePeriodElapsedCallback()
{
    ASSERT(SignalReader::s_instance != nullptr);
    SignalReader::s_instance->isr();
}

// ---------------------------------------------------------------------------------------------- //

SignalReader* SignalReader::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

SignalReader::SignalReader(Owner* owner)
    : m_owner(owner)
{
    ASSERT(owner != nullptr);

    ASSERT(s_instance == nullptr);
    s_instance = this;

    ASSERT(Config::TimerClockFrequency /
           (m_timHandle->Init.Prescaler + 1) /
           (m_timHandle->Init.Period + 1) == OversampleRate);

    // Disable RXONLY mode (no continuous clock wanted)
    m_spiHandle->Instance->CR1 &= ~SPI_CR1_RXONLY;
    __HAL_SPI_ENABLE(m_spiHandle);
}

// ---------------------------------------------------------------------------------------------- //

SignalReader::~SignalReader()
{
    HAL_TIM_Base_Stop_IT(m_timHandle);
    __HAL_SPI_DISABLE(m_spiHandle);

    ASSERT(s_instance == this);
    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::setCalibrationOffsets(int voltage, int current)
{
    ASSERT(voltage >= MinimumCalibrationOffset && voltage <= MaximumCalibrationOffset);
    ASSERT(current >= MinimumCalibrationOffset && current <= MaximumCalibrationOffset);

    m_voltageOffset = voltage;
    m_currentOffset = current;
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::start(Measurement::Gain gain)
{
    ASSERT(m_timHandle->State != HAL_TIM_STATE_BUSY);

    m_gain = Measurement::indexOf(gain);
    m_voltage = 0;
    m_current = 0;

    m_oversampleCounter = 0;
    m_sampleCounter = 0;

    startNextConversion();

    __HAL_TIM_SET_COUNTER(m_timHandle, 0);
    HAL_TIM_Base_Start_IT(m_timHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::stop()
{
    ASSERT(m_timHandle->State == HAL_TIM_STATE_BUSY);

    m_stopFlag = true;

    while (m_stopFlag)
        continue;
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::update()
{
    if (m_samplesReady)
    {
        m_samplesReady = false;
        m_owner->onSamplesAvailable(*m_transferBuffer);
    }
}

// ---------------------------------------------------------------------------------------------- //

inline
void SignalReader::startNextConversion()
{
    HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ADC_CONVST_GPIO_Port, ADC_CONVST_Pin, GPIO_PIN_SET);
}

// ---------------------------------------------------------------------------------------------- //

inline
auto SignalReader::readSamples() -> std::array<uint16_t, ChannelCount>
{
    std::array<uint16_t, ChannelCount> samples;

    // Clear spurious data
    if (__HAL_SPI_GET_FLAG(m_spiHandle, SPI_FLAG_RXNE) == SET)
        m_spiHandle->Instance->DR;

    for (size_t i = 0; i < ChannelCount; ++i)
    {
        // Select ADC
        HAL_GPIO_WritePin(m_csPorts[i], m_csPins[i], GPIO_PIN_RESET);

        // Send dummy data to generate clock
        m_spiHandle->Instance->DR = 0x0000;

        // Wait for transmission to complete
        while (__HAL_SPI_GET_FLAG(m_spiHandle, SPI_FLAG_RXNE) == RESET)
            continue;

        // Read data
        samples[i] = m_spiHandle->Instance->DR;

        // Release ADC
        HAL_GPIO_WritePin(m_csPorts[i], m_csPins[i], GPIO_PIN_SET);
    }

    return samples;
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::isr()
{
    static constexpr int MaximumValue = (1<<16) - 1;

    auto samples = readSamples();
    startNextConversion();

    m_voltage += MaximumValue - samples[0];
    m_current +=                samples[1];

    if (++m_oversampleCounter >= OversampleFactor)
    {
        m_voltage += m_voltageOffset;
        m_current += m_currentOffset;

        (*m_workingBuffer)[m_sampleCounter] = { m_gain, m_voltage, m_current };

        m_voltage = 0;
        m_current = 0;

        m_oversampleCounter = 0;

        if (++m_sampleCounter >= SamplesPerTransfer)
        {
            std::swap(m_workingBuffer, m_transferBuffer);

            m_sampleCounter = 0;
            m_samplesReady = true;
        }

        if (m_stopFlag)
        {
            HAL_TIM_Base_Stop_IT(m_timHandle);
            m_stopFlag = false;
        }
    }
}

// ---------------------------------------------------------------------------------------------- //
