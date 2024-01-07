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
#include "logger.h"
#include "potentiostatnode.h"
#include "potentiostatsensor.h"

// ---------------------------------------------------------------------------------------------- //

PotentiostatNode::PotentiostatNode(const QString& id, const QString& serialPort)
    : Node(id, "potentiostat"),
      m_device(serialPort.toStdString())
{
    m_device.addListener(this);
}

// ---------------------------------------------------------------------------------------------- //

auto PotentiostatNode::alarmThresholds() const -> const Alarm::Thresholds&
{
    static constexpr double OvervoltageWarningThreshold = 5.3;
    static constexpr double OvervoltageCriticalThreshold = 5.4;

    static constexpr double UndervoltageWarningThreshold = 4.9;
    static constexpr double UndervoltageCriticalThreshold = 4.8;

    static constexpr double OvercurrentWarningThreshold = 0.15;
    static constexpr double OvercurrentCriticalThreshold = 0.2;

    static constexpr double OverheatWarningThreshold = 45.0;
    static constexpr double OverheatCriticalThreshold = 55.0;

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

void PotentiostatNode::startMeasurement()
{
    if (m_measurementStarted)
        return;

    startNextMeasurement();
    m_measurementStarted = true;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::stopMeasurement()
{
    if (!m_measurementStarted)
        return;

    m_device.stopMeasurement();
    m_measurementStarted = false;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::update()
{
    if (m_exception)
    {
        auto exception = m_exception;
        m_exception = nullptr;

        std::rethrow_exception(exception);
    }

    m_device.requestPowerValues();

    if (m_measurementComplete)
        handleMeasurementComplete();
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::handlePowerValues(double voltage, double current, double temperature)
{
    const Status status = {
        voltage, current, temperature
    };

    updateStatus(status);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::registerSensor(PotentiostatSensor* sensor, size_t input)
{
    RETURN_IF_NULL(sensor);
    RETURN_IF(input >= InputCount);

    if (m_sensor)
        throw Exception(QString("Input %1 of node %2 is already used.").arg(input).arg(id()));

    m_sensor = sensor;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::unregisterSensor(PotentiostatSensor* sensor)
{
    RETURN_IF_NOT(m_sensor == sensor);
    m_sensor = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::setup(const Device::Setup& setup, const Device::Calibration& calibration)
{
    m_setup = setup;
    m_device.setCalibration(calibration);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::startNextMeasurement()
{
    RETURN_IF(m_measurementRunning);

    m_data.voltage.clear();
    m_data.current.clear();

    m_device.startMeasurement(m_setup);
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::handleMeasurementComplete()
{
    RETURN_IF_NOT(m_measurementComplete);

    m_measurementComplete = false;
    Logger::info("Potentiostat " + id() + " completed measurement.");

    if (m_sensor)
        m_sensor->processData(m_data);

    if (m_measurementStarted)
        startNextMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onMeasurementStarted() noexcept
{
    m_measurementRunning = true;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onMeasurementStopped() noexcept
{
    m_measurementRunning = false;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onMeasurementComplete() noexcept
{
    RETURN_IF(m_measurementComplete);

    m_measurementComplete = true;
    m_measurementRunning = false;
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onCurrentRangeChanged(Device::CurrentRange range) noexcept
{
    static const QString msg = "Current range for potentiostat %1 changed to %2.";
    Logger::info(msg.arg(id(), Device::toString(range).c_str()));
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onSamplesReceived(std::span<const double> voltages,
                                         std::span<const double> currents) noexcept
{
    m_data.voltage.reserve(m_data.voltage.size() + voltages.size());
    m_data.current.reserve(m_data.current.size() + currents.size());

    std::copy(voltages.begin(), voltages.end(), std::back_inserter(m_data.voltage));
    std::copy(currents.begin(), currents.end(), std::back_inserter(m_data.current));
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onPowerValuesReceived(const Device::PowerValues& values) noexcept
{
    QMetaObject::invokeMethod(this, "handlePowerValues", Qt::QueuedConnection,
                              Q_ARG(double, values.voltage),
                              Q_ARG(double, values.current),
                              Q_ARG(double, values.temperature));
}

// ---------------------------------------------------------------------------------------------- //

void PotentiostatNode::onError(const std::string& msg) noexcept
{
    m_exception = std::make_exception_ptr(Exception(msg));
}

// ---------------------------------------------------------------------------------------------- //
