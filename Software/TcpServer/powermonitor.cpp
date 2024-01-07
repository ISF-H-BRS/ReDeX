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

#include "logger.h"
#include "powermonitor.h"
#include "protocol.h"

#include <fstream>

// ---------------------------------------------------------------------------------------------- //

PowerMonitor::PowerMonitor(const DeviceManager& devices)
    : m_devices(devices),
      m_timer(this)
{
    m_timer.setInterval(1s);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::start()
{
    m_timer.start();
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::stop()
{
    m_timer.stop();
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::setNotificationEnabled(bool enable)
{
    m_notificationEnabled = enable;
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::update()
{
    const std::vector<Node*>& nodes = m_devices.nodes();

    size_t maxSeverity = Alarm::indexOf(Alarm::Severity::None);

    for (auto node : nodes)
    {
        const size_t severity = checkAlarmThresholds(*node);

        if (severity > maxSeverity)
            maxSeverity = severity;
    }

    if (maxSeverity != m_currentSeverity)
    {
        m_currentSeverity = maxSeverity;
        emit alarmStatusChanged(m_currentSeverity);
    }

    if (m_notificationEnabled)
    {
        processHubStatus();

        const std::vector<Node*>& nodes = m_devices.nodes();

        for (auto node : nodes)
            processNodeStatus(*node);
    }
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::processHubStatus()
{
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");

    int temperature = 0;
    file >> temperature;

    const QString record = Protocol::joinTokens("<HUB_STATUS>", "%1");
    emit recordAvailable(record.arg(temperature * 0.001));
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::processNodeStatus(const Node& node)
{
    const QString& id = node.id();
    const Node::Status& status = node.currentStatus();

    const double voltage = status.voltage;
    const double current = status.current;
    const double temperature = status.temperature;

    const QString values = Protocol::joinValues("%2", "%3", "%4");
    const QString record = Protocol::joinTokens("<NODE_STATUS>", "%1", values);

    emit recordAvailable(record.arg(id).arg(voltage).arg(current).arg(temperature));
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::checkAlarmThresholds(const Node& node) -> size_t
{
    static constexpr double VoltageHysteresis = 0.1;
    static constexpr double CurrentHysteresis = 0.02;
    static constexpr double HeatHysteresis = 2.0;

    const QString& id = node.id();

    const Node::Status& currentStatus = node.currentStatus();
    const Node::Status& lastStatus = node.lastStatus();

    const Alarm::Thresholds& thresholds = node.alarmThresholds();

    size_t maxSeverity = Alarm::indexOf(Alarm::Severity::None);

    const auto updateSeverity = [&](Alarm::Severity severity)
    {
        const size_t index = Alarm::indexOf(severity);

        if (index > maxSeverity)
            maxSeverity = index;
    };

    const auto logWarning = [&](const QString& type) {
        Logger::warning("A " + type + " condition was detected for node " + id + ".");
    };

    const auto logError = [&](const QString& type) {
        Logger::error("A critical " + type + " condition was detected for node " + id + ".");
    };

    const auto emitAlarm = [&](Alarm::Type type, Alarm::Severity severity)
    {
        const QString values = Protocol::joinValues("%2", "%3");
        const QString record = Protocol::joinTokens("<NODE_ALARM>", "%1", values);

        emit recordAvailable(record.arg(id, Alarm::toString(type), Alarm::toString(severity)));
    };

    const bool isCriticalOvervoltage =
            currentStatus.voltage >= thresholds.overvoltageCritical;

    const bool isCriticalUndervoltage =
            currentStatus.voltage <= thresholds.undervoltageCritical;

    const bool isWarningOvervoltage =
            currentStatus.voltage >= thresholds.overvoltageWarning;

    const bool isWarningUndervoltage =
            currentStatus.voltage <= thresholds.undervoltageWarning;

    if (isCriticalOvervoltage)
    {
        updateSeverity(Alarm::Severity::Critical);

        const bool isNewCriticalOvervoltage =
                lastStatus.voltage < thresholds.overvoltageCritical - VoltageHysteresis;

        if (isNewCriticalOvervoltage)
        {
            logError("overvoltage");
            emitAlarm(Alarm::Type::Overvoltage, Alarm::Severity::Critical);
        }
    }
    else if (isCriticalUndervoltage)
    {
        updateSeverity(Alarm::Severity::Critical);

        const bool isNewCriticalUndervoltage =
                lastStatus.voltage > thresholds.undervoltageCritical + VoltageHysteresis;

        if (isNewCriticalUndervoltage)
        {
            logError("undervoltage");
            emitAlarm(Alarm::Type::Undervoltage, Alarm::Severity::Critical);
        }
    }
    else if (isWarningOvervoltage)
    {
        updateSeverity(Alarm::Severity::Warning);

        const bool isNewWarningOvervoltage =
                lastStatus.voltage < thresholds.overvoltageWarning - VoltageHysteresis;

        if (isNewWarningOvervoltage)
        {
            logWarning("overvoltage");
            emitAlarm(Alarm::Type::Overvoltage, Alarm::Severity::Warning);
        }
    }
    else if (isWarningUndervoltage)
    {
        updateSeverity(Alarm::Severity::Warning);

        const bool isNewWarningUndervoltage =
                lastStatus.voltage > thresholds.undervoltageWarning + VoltageHysteresis;

        if (isNewWarningUndervoltage)
        {
            logWarning("undervoltage");
            emitAlarm(Alarm::Type::Undervoltage, Alarm::Severity::Warning);
        }
    }

    const bool isCriticalOvercurrent =
            currentStatus.current >= thresholds.overcurrentCritical;

    const bool isWarningOvercurrent =
            currentStatus.current >= thresholds.overcurrentWarning;

    if (isCriticalOvercurrent)
    {
        updateSeverity(Alarm::Severity::Critical);

        const bool isNewCriticalOvercurrent =
                lastStatus.current < thresholds.overcurrentCritical - CurrentHysteresis;

        if (isNewCriticalOvercurrent)
        {
            logError("overcurrent");
            emitAlarm(Alarm::Type::Overcurrent, Alarm::Severity::Critical);
        }
    }
    else if (isWarningOvercurrent)
    {
        updateSeverity(Alarm::Severity::Warning);

        const bool isNewWarningOvercurrent =
                lastStatus.current < thresholds.overcurrentWarning - CurrentHysteresis;

        if (isNewWarningOvercurrent)
        {
            logWarning("overcurrent");
            emitAlarm(Alarm::Type::Overcurrent, Alarm::Severity::Warning);
        }
    }

    const bool isCriticalOverheat =
            currentStatus.temperature >= thresholds.overheatCritical;

    const bool isWarningOverheat =
            currentStatus.temperature >= thresholds.overheatWarning;

    if (isCriticalOverheat)
    {
        updateSeverity(Alarm::Severity::Critical);

        const bool isNewCriticalOverheat =
                lastStatus.temperature < thresholds.overheatCritical - HeatHysteresis;

        if (isNewCriticalOverheat)
        {
            logError("overheat");
            emitAlarm(Alarm::Type::Overheat, Alarm::Severity::Critical);
        }
    }
    else if (isWarningOverheat)
    {
        updateSeverity(Alarm::Severity::Warning);

        const bool isNewWarningOverheat =
                lastStatus.temperature < thresholds.overheatWarning - HeatHysteresis;

        if (isNewWarningOverheat)
        {
            logWarning("overheat");
            emitAlarm(Alarm::Type::Overheat, Alarm::Severity::Warning);
        }
    }

    return maxSeverity;
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::emitHubAlarm(Alarm::Type type, Alarm::Severity severity)
{
    const QString values = Protocol::joinValues("%1", "%2");
    const QString record = Protocol::joinTokens("<HUB_ALARM>", values);

    emit recordAvailable(record.arg(Alarm::toString(type), Alarm::toString(severity)));
}

// ---------------------------------------------------------------------------------------------- //
