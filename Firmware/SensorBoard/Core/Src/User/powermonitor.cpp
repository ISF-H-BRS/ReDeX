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
#include "config.h"
#include "powermonitor.h"

// ---------------------------------------------------------------------------------------------- //

PowerMonitor::PowerMonitor()
{
    HAL_ADCEx_Calibration_Start(m_adcHandle, ADC_SINGLE_ENDED);
    startNextConversion();
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::update()
{
    static constexpr double ReferenceVoltage = 1.212;

    if (HAL_ADC_PollForConversion(m_adcHandle, 1) == HAL_OK)
    {
        const uint16_t voltageRaw     = m_data[indexOf(Channel::Voltage)];
        const uint16_t currentRaw     = m_data[indexOf(Channel::Current)];
        const uint16_t temperatureRaw = m_data[indexOf(Channel::Temperature)];
        const uint16_t referenceRaw   = m_data[indexOf(Channel::Reference)];

        const double voltage     = toVoltage(ReferenceVoltage / referenceRaw * voltageRaw);
        const double current     = toCurrent(ReferenceVoltage / referenceRaw * currentRaw);
        const double temperature = toTemperature(temperatureRaw);

        m_voltage.addSample(voltage);
        m_current.addSample(current);
        m_temperature.addSample(temperature);

        startNextConversion();
    }
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getVoltage() const -> double
{
    return m_voltage.getValue();
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getCurrent() const -> double
{
    return m_current.getValue();
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getTemperature() const -> double
{
    return m_temperature.getValue();
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::startNextConversion()
{
    auto data = reinterpret_cast<uint32_t*>(const_cast<uint16_t*>(m_data.data()));
    HAL_ADC_Start_DMA(m_adcHandle, data, ChannelCount);
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::toVoltage(double value) -> double
{
    static constexpr double VoltageFactor = 2.0;
    return VoltageFactor * value;
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::toCurrent(double value) -> double
{
    static constexpr double CurrentToVoltageResistance = 7870.0;
    static constexpr double ShuntResistance = 0.18;

    return value / (CurrentToVoltageResistance * 0.004) / ShuntResistance;
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::toTemperature(uint16_t rawValue) -> double
{
    static constexpr double ReferenceVoltage = 3.3;
    static constexpr double CalibrationVoltage = 3.0;
    static constexpr double CalibrationFactor = ReferenceVoltage / CalibrationVoltage;

    static constexpr double JunctionToAmbient = 5.0; // Rough estimate

    static constexpr uintptr_t TsCal1Address = 0x1fff75a8;
    static constexpr uintptr_t TsCal2Address = 0x1fff75ca;

    static constexpr double TsCal1Temperature =  30.0;
    static constexpr double TsCal2Temperature = 130.0;

    static constexpr int OversampleShift = 4; // 16 bits -> 12 bits

    static const double tsCal1Value = *reinterpret_cast<uint16_t*>(TsCal1Address);
    static const double tsCal2Value = *reinterpret_cast<uint16_t*>(TsCal2Address);

    static const double tsCalRatio =
            (TsCal2Temperature - TsCal1Temperature) / (tsCal2Value - tsCal1Value);

    const double value = (rawValue >> OversampleShift) * CalibrationFactor;

    return (value - tsCal1Value) * tsCalRatio + TsCal1Temperature - JunctionToAmbient;
}

// ---------------------------------------------------------------------------------------------- //
