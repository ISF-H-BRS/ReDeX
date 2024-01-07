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

#include "config.h"
#include "measurement.h"

#include <array>
#include <span>

class SignalReader
{
public:
    static constexpr size_t ChannelCount = 2;
    static constexpr auto SamplesPerTransfer = Config::SamplesPerTransfer;

    static constexpr uint32_t SampleRate = 1000;

    static constexpr int MinimumCalibrationOffset = -8192;
    static constexpr int MaximumCalibrationOffset = +8192;

    class Owner
    {
    public:
        virtual void onSamplesAvailable(std::span<const Measurement::Sample> samples) = 0;
    };

public:
    SignalReader(Owner* owner);
    ~SignalReader();

    void setCalibrationOffsets(int voltage, int current);

    void start(Measurement::Gain gain);
    void stop();
    void update();

private:
    friend void SamplePeriodElapsedCallback();
    void isr();

    void startNextConversion();
    auto readSamples() -> std::array<uint16_t, ChannelCount>;

private:
    static constexpr uint32_t OversampleBits = 4;
    static constexpr uint32_t OversampleFactor = (1<<OversampleBits);
    static constexpr uint32_t OversampleRate = OversampleFactor * SampleRate;

private:
    Owner* m_owner;

    SPI_HandleTypeDef* m_spiHandle = Config::AdcSpiHandle;
    TIM_HandleTypeDef* m_timHandle = Config::SampleTimerHandle;

    std::array<GPIO_TypeDef*, ChannelCount> m_csPorts = { ADC_CS1_GPIO_Port, ADC_CS2_GPIO_Port };
    std::array<uint16_t, ChannelCount> m_csPins = { ADC_CS1_Pin, ADC_CS2_Pin };

    int m_gain = Measurement::indexOf(Measurement::DefaultGain);
    int m_voltage = 0;
    int m_current = 0;

    int m_voltageOffset = 0;
    int m_currentOffset = 0;

    uint32_t m_oversampleCounter = 0;

    using SampleBuffer = std::array<Measurement::Sample, SamplesPerTransfer>;

    SampleBuffer m_sampleBuffer0;
    SampleBuffer m_sampleBuffer1;

    SampleBuffer* m_workingBuffer = &m_sampleBuffer0;
    SampleBuffer* m_transferBuffer = &m_sampleBuffer1;

    uint32_t m_sampleCounter = 0;

    volatile bool m_samplesReady = false;
    volatile bool m_stopFlag = false;

    static SignalReader* s_instance;
};
