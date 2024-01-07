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

#include "tcpsocket.h"

#include <redex.h>

#include <thread>

namespace redex {

class TcpClient
{
public:
    TcpClient(const std::string& host, Listener* listener);
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient(TcpClient&&) = delete;

    auto operator=(const TcpClient&) = delete;
    auto operator=(TcpClient&&) = delete;

    void requestNodeInfo();
    void requestTestpointInfo();

    void startPowerMonitor();
    void stopPowerMonitor();

    void startMeasurement();
    void stopMeasurement();

private:
    void waitForPreamble();

    void work();

    void processData(std::span<const char> data);

    auto parseSensorInfo(std::span<const std::string> values) -> SensorInfo;

    void parseNodeInfo(std::span<const std::string> tokens);
    void parseTestpointInfo(std::span<const std::string> tokens);

    void parseHubAlarm(std::span<const std::string> tokens);
    void parseNodeAlarm(std::span<const std::string> tokens);

    void parseHubStatus(std::span<const std::string> tokens);
    void parseNodeStatus(std::span<const std::string> tokens);

    void parseStatus(std::span<const std::string> tokens);
    void parseError(std::span<const std::string> tokens);

    void parseVoltammogram(std::span<const std::string> tokens);
    void parseSensorData(std::span<const std::string> tokens);

    void sendData(const std::string& data);

private:
    TcpSocket m_socket;
    Listener* m_listener;

    std::thread m_thread;
    bool m_running = false;

    std::string m_currentData;
};

} // End of namespace redex
