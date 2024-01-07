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
#include "exception.h"
#include "sensorsnode.h"
#include "sensorssensor.h"

// ---------------------------------------------------------------------------------------------- //

SensorsNode::SensorsNode(const QString& id, const QString& serialPort)
    : Node(id, "sensors"),
      m_device(serialPort.toStdString())
{
}

// ---------------------------------------------------------------------------------------------- //

auto SensorsNode::getSensorType(size_t index) const -> Device::SensorType
{
    return m_device.getSensorType(index);
}

// ---------------------------------------------------------------------------------------------- //

auto SensorsNode::alarmThresholds() const -> const Alarm::Thresholds&
{
    static constexpr double OvervoltageWarningThreshold = 5.3;
    static constexpr double OvervoltageCriticalThreshold = 5.4;

    static constexpr double UndervoltageWarningThreshold = 4.8;
    static constexpr double UndervoltageCriticalThreshold = 4.7;

    static constexpr double OvercurrentWarningThreshold = 0.15;
    static constexpr double OvercurrentCriticalThreshold = 0.2;

    static constexpr double OverheatWarningThreshold = 40.0;
    static constexpr double OverheatCriticalThreshold = 50.0;

    static const Alarm::Thresholds thresholds = {
        OvervoltageWarningThreshold,
        OvervoltageCriticalThreshold,
        UndervoltageWarningThreshold,
        UndervoltageCriticalThreshold,
        OvercurrentWarningThreshold,
        OvercurrentCriticalThreshold,
        OverheatWarningThreshold,
        OverheatCriticalThreshold
    };

    return thresholds;
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::startMeasurement()
{
    m_measurementStarted = true;
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::stopMeasurement()
{
    m_measurementStarted = false;
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::update()
{
    updateStatus();

    if (m_measurementStarted)
        updateMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::registerSensor(SensorsSensor* sensor, size_t input)
{
    RETURN_IF_NULL(sensor);
    RETURN_IF(input >= InputCount);

    if (m_sensors[input])
        throw Exception(QString("Input %1 of node %2 is already used.").arg(input).arg(id()));

    m_sensors[input] = sensor;
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::unregisterSensor(SensorsSensor* sensor)
{
    for (auto ptr : m_sensors)
    {
        if (ptr == sensor)
            ptr = nullptr;
    }
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::updateStatus()
{
    const Device::PowerValues values = m_device.getPowerValues();

    const Status status = {
        values.voltage, values.current, values.temperature
    };

    Node::updateStatus(status);
}

// ---------------------------------------------------------------------------------------------- //

void SensorsNode::updateMeasurement()
{
    RETURN_IF_NOT(m_measurementStarted);

    for (size_t i = 0; i < InputCount; ++i)
    {
        if (m_sensors[i])
            m_sensors[i]->processValue(m_device.getSensorValue(i));
    }
}

// ---------------------------------------------------------------------------------------------- //
