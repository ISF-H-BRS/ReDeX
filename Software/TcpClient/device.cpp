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

#include "device.h"

// ---------------------------------------------------------------------------------------------- //

using namespace std::string_literals;

// ---------------------------------------------------------------------------------------------- //

Device::Device(const QString& hostname)
    : m_client(hostname.toStdString(), this) {}

// ---------------------------------------------------------------------------------------------- //

void Device::requestInfo()
{
    m_client.requestNodeInfo();
    m_client.requestTestpointInfo();
}

// ---------------------------------------------------------------------------------------------- //

void Device::startPowerMonitor()
{
    m_client.startPowerMonitor();
}

// ---------------------------------------------------------------------------------------------- //

void Device::stopPowerMonitor()
{
    m_client.stopPowerMonitor();
}

// ---------------------------------------------------------------------------------------------- //

void Device::startMeasurement()
{
    m_client.startMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Device::stopMeasurement()
{
    m_client.stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Device::onNodeInfoReceived(std::span<const redex::NodeInfo> info)
{
    const std::vector<redex::NodeInfo> infos(info.begin(), info.end());

    QMetaObject::invokeMethod(this, "nodeInfoReceived", Qt::QueuedConnection,
                              Q_ARG(std::vector<redex::NodeInfo>, infos));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onTestpointInfoReceived(std::span<const redex::TestpointInfo> info)
{
    const std::vector<redex::TestpointInfo> infos(info.begin(), info.end());

    QMetaObject::invokeMethod(this, "testpointInfoReceived", Qt::QueuedConnection,
                              Q_ARG(std::vector<redex::TestpointInfo>, infos));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onStatusChanged(redex::Status status)
{
    QMetaObject::invokeMethod(this, "statusChanged", Qt::QueuedConnection,
                              Q_ARG(redex::Status, status));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onConductanceReceived(const std::string& testpointId,
                                   double voltage, double current, double admittance)
{
    QMetaObject::invokeMethod(this, "conductanceReceived", Qt::QueuedConnection,
                              Q_ARG(QString, testpointId.c_str()),
                              Q_ARG(double, voltage),
                              Q_ARG(double, current),
                              Q_ARG(double, admittance));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onOrpValueReceived(const std::string& testpointId, double value)
{
    QMetaObject::invokeMethod(this, "orpValueReceived", Qt::QueuedConnection,
                              Q_ARG(QString, testpointId.c_str()),
                              Q_ARG(double, value));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onPhValueReceived(const std::string& testpointId, double value)
{
    QMetaObject::invokeMethod(this, "phValueReceived", Qt::QueuedConnection,
                              Q_ARG(QString, testpointId.c_str()),
                              Q_ARG(double, value));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onTemperatureReceived(const std::string& testpointId, double value)
{
    QMetaObject::invokeMethod(this, "temperatureReceived", Qt::QueuedConnection,
                              Q_ARG(QString, testpointId.c_str()),
                              Q_ARG(double, value));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onVoltammogramReceived(const std::string& testpointId,
                                    std::span<const double> voltage,
                                    std::span<const double> current)
{
    static const auto toVector = [](std::span<const double> span) {
        return std::vector<double>(span.begin(), span.end());
    };

    const Voltammogram voltammogram = {
        toVector(voltage),
        toVector(current)
    };

    QMetaObject::invokeMethod(this, "voltammogramReceived", Qt::QueuedConnection,
                              Q_ARG(QString, testpointId.c_str()),
                              Q_ARG(Device::Voltammogram, voltammogram));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onHubAlarm(redex::AlarmType type, redex::AlarmSeverity severity)
{
    QMetaObject::invokeMethod(this, "hubAlarm", Qt::QueuedConnection,
                              Q_ARG(redex::AlarmType, type),
                              Q_ARG(redex::AlarmSeverity, severity));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onNodeAlarm(const std::string& nodeId,
                         redex::AlarmType type, redex::AlarmSeverity severity)
{
    QMetaObject::invokeMethod(this, "hubAlarm", Qt::QueuedConnection,
                              Q_ARG(QString, nodeId.c_str()),
                              Q_ARG(redex::AlarmType, type),
                              Q_ARG(redex::AlarmSeverity, severity));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onHubStatusReceived(double temperature)
{
    QMetaObject::invokeMethod(this, "hubStatusReceived", Qt::QueuedConnection,
                              Q_ARG(double, temperature));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onNodeStatusReceived(const std::string& nodeId,
                                  double voltage, double current, double temperature)
{
    QMetaObject::invokeMethod(this, "nodeStatusReceived", Qt::QueuedConnection,
                              Q_ARG(QString, nodeId.c_str()),
                              Q_ARG(double, voltage),
                              Q_ARG(double, current),
                              Q_ARG(double, temperature));
}

// ---------------------------------------------------------------------------------------------- //

void Device::onError(const std::string& msg)
{
    QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                              Q_ARG(QString, msg.c_str()));
}

// ---------------------------------------------------------------------------------------------- //
