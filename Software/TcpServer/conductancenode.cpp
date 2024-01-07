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
#include "exception.h"

// ---------------------------------------------------------------------------------------------- //

ConductanceNode::ConductanceNode(const QString& id, const QString& serial)
    : Node(id, "conductance")
{
    const std::vector<DeviceInfo> devices = DeviceInfo::getAvailableDevices();

    for (const auto& info : devices)
    {
        if (info.serialNumber() == serial.toStdString())
        {
            m_device = std::make_unique<Device>(info);
            m_device->addListener(this);
            break;
        }
    }

    if (!m_device)
        throw Exception("Conductance node with serial number " + serial + " not found.");
}

// ---------------------------------------------------------------------------------------------- //

auto ConductanceNode::alarmThresholds() const -> const Alarm::Thresholds&
{
    static constexpr double OvervoltageWarningThreshold = 5.3;
    static constexpr double OvervoltageCriticalThreshold = 5.4;

    static constexpr double UndervoltageWarningThreshold = 4.8;
    static constexpr double UndervoltageCriticalThreshold = 4.7;

    static constexpr double OvercurrentWarningThreshold = 0.25;
    static constexpr double OvercurrentCriticalThreshold = 0.3;

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

void ConductanceNode::startMeasurement()
{
    m_device->startCapture();
    m_measurementStarted = true;
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::stopMeasurement()
{
    m_device->stopCapture();
    m_measurementStarted = false;
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::update()
{
    if (m_exception)
    {
        auto exception = m_exception;
        m_exception = nullptr;

        std::rethrow_exception(exception);
    }

    updateStatus();

    if (m_measurementStarted)
        updateMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::registerSensor(ConductanceSensor* sensor, size_t input)
{
    RETURN_IF_NULL(sensor);
    RETURN_IF(input >= InputCount);

    if (m_sensors[input])
        throw Exception(QString("Input %1 of node %2 is already used.").arg(input).arg(id()));

    m_sensors[input] = sensor;
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::unregisterSensor(ConductanceSensor* sensor)
{
    for (auto ptr : m_sensors)
    {
        if (ptr == sensor)
            ptr = nullptr;
    }
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::setupSignal(size_t input, Device::Waveform waveform,
                                  unsigned int frequency, double amplitude)
{
    RETURN_IF(input >= InputCount);
    RETURN_IF_NULL(m_sensors[input]);

    m_device->setupSignal(Device::toInput(input), waveform, frequency, amplitude);
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::setGain(size_t input, Device::Gain gain)
{
    RETURN_IF(input >= InputCount);
    RETURN_IF_NULL(m_sensors[input]);

    m_device->setGain(Device::toInput(input), gain);
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::updateStatus()
{
    const Device::PowerValues values = m_device->getPowerValues();

    const Status status = {
        values.voltage, values.current, values.temperature
    };

    Node::updateStatus(status);
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::updateMeasurement()
{
    RETURN_IF_NOT(m_measurementStarted);

    std::lock_guard lock(m_mutex);

    for (size_t i = 0; i < InputCount; ++i)
    {
        if (m_sensors[i])
            m_sensors[i]->processData(m_dataBuffers[i]);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::onDataAvailable(const Device::Data& data)
{
    std::lock_guard lock(m_mutex);

    for (size_t i = 0; i < InputCount; ++i)
    {
        if (m_sensors[i])
            m_dataBuffers[i].update(data);
    }
}

// ---------------------------------------------------------------------------------------------- //

void ConductanceNode::onError(const std::string& msg)
{
    m_exception = std::make_exception_ptr(Exception(msg));
}

// ---------------------------------------------------------------------------------------------- //
