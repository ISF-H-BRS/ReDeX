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

#ifdef _WIN32
  #ifdef REDEX_BUILD_PROCESS
    #define REDEX_EXPORT __declspec(dllexport)
  #else
    #define REDEX_EXPORT __declspec(dllimport)
  #endif
#else
  #define REDEX_EXPORT __attribute__((visibility("default")))
#endif

#include <memory>
#include <span>
#include <stdexcept>
#include <string>

namespace redex {

struct NodeInfo
{
    std::string id;
    std::string type;
};

struct SensorInfo
{
    std::string sensorId;
    std::string nodeId;
    size_t input;
};

struct TestpointInfo
{
    std::string testpointId;
    SensorInfo conductanceInfo;
    SensorInfo orpInfo;
    SensorInfo phInfo;
    SensorInfo potentiostatInfo;
    SensorInfo temperatureInfo;
};

enum class Status
{
    MeasurementStarted,
    MeasurementStopped,
    MeasurementError
};

enum class AlarmType
{
    Overvoltage,
    Undervoltage,
    Overcurrent,
    Overheat
};

enum class AlarmSeverity
{
    Warning,
    Critical
};

using Error = std::runtime_error;

class REDEX_EXPORT Listener
{
public:
    virtual void onNodeInfoReceived(std::span<const NodeInfo> info);

    virtual void onTestpointInfoReceived(std::span<const TestpointInfo> info);

    virtual void onStatusChanged(Status status);

    virtual void onConductanceReceived(const std::string& testpointId,
                                       double voltage, double current, double admittance);

    virtual void onOrpValueReceived(const std::string& testpointId, double value);

    virtual void onPhValueReceived(const std::string& testpointId, double value);

    virtual void onTemperatureReceived(const std::string& testpointId, double value);

    virtual void onVoltammogramReceived(const std::string& testpointId,
                                        std::span<const double> voltage,
                                        std::span<const double> current);

    virtual void onHubAlarm(AlarmType type, AlarmSeverity severity);

    virtual void onNodeAlarm(const std::string& nodeId, AlarmType type, AlarmSeverity severity);

    virtual void onHubStatusReceived(double temperature);

    virtual void onNodeStatusReceived(const std::string& nodeId,
                                      double voltage, double current, double temperature);

    virtual void onError(const std::string& msg);
};

class Client
{
    Client(const Client&) = delete;
    Client(Client&&) = delete;

    auto operator=(const Client&) = delete;
    auto operator=(Client&&) = delete;

public:
    REDEX_EXPORT Client(const std::string& host, Listener* listener);
    REDEX_EXPORT ~Client();

    REDEX_EXPORT void requestNodeInfo();
    REDEX_EXPORT void requestTestpointInfo();

    REDEX_EXPORT void startPowerMonitor();
    REDEX_EXPORT void stopPowerMonitor();

    REDEX_EXPORT void startMeasurement();
    REDEX_EXPORT void stopMeasurement();

    REDEX_EXPORT static void filterVoltammetryData(std::span<const double> input,
                                                   std::span<double> output);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // End of namespace redex
