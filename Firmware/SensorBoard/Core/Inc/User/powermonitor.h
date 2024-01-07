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

#include "averagingbuffer.h"
#include "config.h"

class PowerMonitor
{
public:
    PowerMonitor();

    void update();

    auto getVoltage() const -> double;
    auto getCurrent() const -> double;
    auto getTemperature() const -> double;

private:
    void startNextConversion();

    auto toVoltage(double value) -> double;
    auto toCurrent(double value) -> double;
    auto toTemperature(uint16_t rawValue) -> double;

private:
    enum class Channel
    {
        Voltage,
        Current,
        Temperature,
        Reference
    };

    static constexpr size_t ChannelCount = 4;

    auto indexOf(Channel channel) { return static_cast<size_t>(channel); }

    std::array<volatile uint16_t, ChannelCount> m_data = {};

    static constexpr size_t AverageCount = 5;

    AveragingBuffer<double, AverageCount> m_voltage;
    AveragingBuffer<double, AverageCount> m_current;
    AveragingBuffer<double, AverageCount> m_temperature;

    ADC_HandleTypeDef* m_adcHandle = Config::PowerMonitorHandle;
};
