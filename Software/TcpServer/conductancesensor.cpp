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

#include "assertions.h"
#include "conductancenode.h"
#include "conductancesensor.h"
#include "logger.h"

// ---------------------------------------------------------------------------------------------- //

ConductanceSensor::ConductanceSensor(const QString& id, ConductanceNode* node, size_t input,
                                     const Configuration& config)
    : Sensor(id, node, input),
      m_node(node)
{
    ASSERT(input < ConductanceNode::InputCount);

    m_node->registerSensor(this, input);
    m_node->setGain(input, m_gain);

    setConfiguration(config);
}

// ---------------------------------------------------------------------------------------------- //

ConductanceSensor::~ConductanceSensor()
{
    m_node->unregisterSensor(this);
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceSensor::setConfiguration(const Configuration& config)
{
    checkConfiguration(config);

    m_node->setupSignal(input(), Device::Waveform::Sine, config.frequency, config.amplitude);
    m_analyzer.setTargetFrequency(config.frequency);
    m_leadResistance = config.leadResistance;
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceSensor::processData(const DataBuffer& buffer)
{
    if (m_analysisSkips > 0)
    {
        --m_analysisSkips;
        return;
    }

    const Analyzer::Magnitudes& magnitudes = m_analyzer.run(buffer);
    auto [voltage, current] = Analyzer::getValues(magnitudes, m_gain, m_leadResistance);

    if (voltage < 1.0e-12)
        voltage = 0.0;

    updateGain(current);

    emit valuesAvailable(voltage, current, current / voltage);
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceSensor::updateGain(double current)
{
    static constexpr double UpperThreshold = 0.95 * Device::MaximumAmplitude;
    static constexpr double LowerThreshold = 0.09 * Device::MaximumAmplitude;

    static constexpr int AnalysisSkips = 2; // Delay for one second to get a full window

    const auto setGain = [this](Device::Gain gain)
    {
        m_gain = gain;
        m_node->setGain(input(), m_gain);
        m_analysisSkips = AnalysisSkips;

        const QString msg = "Gain of conductance sensor %1 set to %2.";
        Logger::info(msg.arg(id(), Device::toString(gain)));
    };

    const auto gain = Device::indexOf(m_gain);
    const auto currentMagnitude = current * Device::toDouble(m_gain);

    if (currentMagnitude > UpperThreshold)
    {
        if (gain > 0)
            setGain(Device::toGain(gain - 1));
    }
    else if (currentMagnitude < LowerThreshold)
    {
        if (gain < Device::GainCount - 1)
            setGain(Device::toGain(gain + 1));
    }
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceSensor::checkConfiguration(const Configuration& config)
{
    if (outOfRange(config.frequency, Device::MinimumFrequency, Device::MaximumFrequency))
        throw Exception(QString::number(config.frequency) + " is not a valid frequency.");

    if (outOfRange(config.amplitude, Device::MinimumAmplitude, Device::MaximumAmplitude))
        throw Exception(QString::number(config.amplitude) + " is not a valid amplitude.");

    if (config.leadResistance < 0.0)
        throw Exception(QString::number(config.leadResistance) + " is not a valid resistance.");
}

// ---------------------------------------------------------------------------------------------- //
