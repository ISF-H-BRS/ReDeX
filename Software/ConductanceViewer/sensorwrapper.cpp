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

#include "sensorwrapper.h"

// ---------------------------------------------------------------------------------------------- //

using namespace std::chrono_literals;

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr std::chrono::milliseconds DefaultTimeout =  500ms;
    constexpr std::chrono::milliseconds DelayedTimeout = 1200ms;
}

// ---------------------------------------------------------------------------------------------- //

SensorWrapper::SensorWrapper(Device* device, Device::Input input, DataBuffer* buffer)
    : m_device(device),
      m_input(input),
      m_dataBuffer(buffer),
      m_analysisTimer(this)
{
    m_analysisTimer.setSingleShot(true);
    connect(&m_analysisTimer, SIGNAL(timeout()), this, SLOT(analyzeData()));

    restartAnalysisTimer(DelayedTimeout);
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::setupSignal(Device::Waveform waveform, unsigned int frequency, double amplitude)
{
    m_analyzer.setTargetFrequency(frequency);
    m_device->setupSignal(m_input, waveform, frequency, amplitude);

    restartAnalysisTimer(DelayedTimeout);
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::setGain(Device::Gain gain)
{
    m_gain = gain;
    m_device->setGain(m_input, gain);

    restartAnalysisTimer(DelayedTimeout);
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::setLeadResistance(unsigned int milliohm) noexcept
{
    m_leadResistance = milliohm * 0.001;
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::update(const Device::Data& data)
{
    m_dataBuffer->update(data);
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::analyzeData()
{
    restartAnalysisTimer(DefaultTimeout);

    const Analyzer::Magnitudes& magnitudes = m_analyzer.run(*m_dataBuffer);
    auto [voltage, current] = Analyzer::getValues(magnitudes, m_gain, m_leadResistance);

    if (voltage < 1.0e-12)
        voltage = 0.0;

    emit analysisComplete(m_input, voltage, current, current / voltage);
}

// ---------------------------------------------------------------------------------------------- //

void SensorWrapper::restartAnalysisTimer(std::chrono::milliseconds ms)
{
    m_analysisTimer.stop();
    m_analysisTimer.start(ms);
}

// ---------------------------------------------------------------------------------------------- //
