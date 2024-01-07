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

#include <redex.h>

#include <QObject>
#include <QString>

#include <map>
#include <stdexcept>

class Device : public QObject, public redex::Listener
{
    Q_OBJECT

public:
    using Error = std::runtime_error;

    struct Voltammogram
    {
        std::vector<double> voltage;
        std::vector<double> current;
    };

public:
    explicit Device(const QString& hostname);

    void requestInfo();
    void requestStatus();

    void startPowerMonitor();
    void stopPowerMonitor();

    void startMeasurement();
    void stopMeasurement();

signals:
    void nodeInfoReceived(const std::vector<redex::NodeInfo>& info);

    void testpointInfoReceived(const std::vector<redex::TestpointInfo>& info);

    void statusChanged(redex::Status status);

    void conductanceReceived(const QString& testpointId,
                             double voltage, double current, double admittance);

    void orpValueReceived(const QString& testpointId, double value);

    void phValueReceived(const QString& testpointId, double value);

    void temperatureReceived(const QString& testpointId, double value);

    void voltammogramReceived(const QString& testpointId, const Device::Voltammogram& voltammogram);

    void hubAlarm(redex::AlarmType type, redex::AlarmSeverity severity);

    void nodeAlarm(const QString& nodeId,
                   redex::AlarmType type, redex::AlarmSeverity severity);

    void hubStatusReceived(double temperature);

    void nodeStatusReceived(const QString& nodeId,
                            double voltage, double current, double temperature);

    void error(const QString& msg);

private:
    void onNodeInfoReceived(std::span<const redex::NodeInfo> info) override;

    void onTestpointInfoReceived(std::span<const redex::TestpointInfo> info) override;

    void onStatusChanged(redex::Status status) override;

    void onConductanceReceived(const std::string& testpointId,
                               double voltage, double current, double admittance) override;

    void onOrpValueReceived(const std::string& testpointId, double value) override;

    void onPhValueReceived(const std::string& testpointId, double value) override;

    void onTemperatureReceived(const std::string& testpointId, double value) override;

    void onVoltammogramReceived(const std::string& testpointId,
                                std::span<const double> voltage,
                                std::span<const double> current) override;

    void onHubAlarm(redex::AlarmType type, redex::AlarmSeverity severity) override;

    void onNodeAlarm(const std::string& nodeId,
                     redex::AlarmType type, redex::AlarmSeverity severity) override;

    void onHubStatusReceived(double temperature) override;

    void onNodeStatusReceived(const std::string& nodeId,
                              double voltage, double current, double temperature) override;

    void onError(const std::string& msg) override;

private:
    redex::Client m_client;
};
