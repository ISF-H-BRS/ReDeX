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

#include "signalgenerator.h"
#include "wavegenerator.h"

#include <algorithm> // for clamp()

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr uint32_t DacZeroChannel   = DAC_CHANNEL_1;
    constexpr uint32_t DacSignalChannel = DAC_CHANNEL_2;
    constexpr uint32_t DacDcOffset      = 2047;
    constexpr uint32_t DacMaximumValue  = 4095;
}

// ---------------------------------------------------------------------------------------------- //

SignalGenerator::SignalGenerator()
{
    HAL_DAC_Start(m_dacHandle, DacZeroChannel);
    HAL_DAC_Start(m_dacHandle, DacSignalChannel);

    setValue(DacZeroChannel, DacDcOffset);
    setValue(DacSignalChannel, DacDcOffset);
}

// ---------------------------------------------------------------------------------------------- //

SignalGenerator::~SignalGenerator()
{
    HAL_DAC_Stop(m_dacHandle, DacZeroChannel);
    HAL_DAC_Stop(m_dacHandle, DacSignalChannel);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::reset()
{
    m_waveform = Waveform::None;
    m_frequency = 1.0;
    m_amplitude = 0.0;

    m_signalPosition = 0;

    setValue(DacSignalChannel, DacDcOffset);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::setup(Waveform waveform, unsigned int frequency, Real amplitude)
{
    frequency = std::clamp(frequency, MinimumFrequency, MaximumFrequency);
    amplitude = std::clamp(amplitude, MinimumAmplitude, MaximumAmplitude);

    m_waveform = waveform;
    m_frequency = static_cast<Real>(frequency);
    m_amplitude = 0.5 / MaximumAmplitude * amplitude;
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::update()
{
    using WaveGenerator = WaveGenerator<Real>;

    static constexpr auto DcOffset = static_cast<Real>(0.5);
    static constexpr auto One = static_cast<Real>(1.0);
    static constexpr auto Zero = static_cast<Real>(0.0);

    const Real time = m_signalPosition * (One / SampleRate);

    if (++m_signalPosition >= SampleRate)
        m_signalPosition = 0;

    Real signal = DcOffset;

    if (m_waveform == Waveform::Sine)
        signal = WaveGenerator::qsine(time, m_frequency, m_amplitude, DcOffset);
    else if (m_waveform == Waveform::Square)
        signal = WaveGenerator::square(time, m_frequency, m_amplitude, DcOffset);
    else if (m_waveform == Waveform::Triangle)
        signal = WaveGenerator::triangle(time, m_frequency, m_amplitude, DcOffset);
    else if (m_waveform == Waveform::Sawtooth)
        signal = WaveGenerator::sawtooth(time, m_frequency, m_amplitude, DcOffset);

    signal = std::clamp(signal, Zero, One);

    const uint16_t outValue = static_cast<uint16_t>(DacMaximumValue * signal);
    setValue(DacSignalChannel, outValue);
}

// ---------------------------------------------------------------------------------------------- //

void SignalGenerator::setValue(uint32_t channel, uint32_t value)
{
    HAL_DAC_SetValue(m_dacHandle, channel, DAC_ALIGN_12B_R, value);
}

// ---------------------------------------------------------------------------------------------- //
