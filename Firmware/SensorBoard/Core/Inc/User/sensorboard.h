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

#include "hostinterface.h"
#include "powermonitor.h"
#include "sensormanager.h"

class SensorBoard : public HostInterface::Owner
{
public:
    SensorBoard();
    ~SensorBoard();

    void exec();

private:
    void onHostDataReceived(const String& data) override;
    void onHostDataOverflow() override;

    void protocolGetSensorType(const String& data, size_t tokenCount);
    void protocolGetSensorValue(const String& data, size_t tokenCount);

    void protocolGetPowerValues();

    void protocolGetBoardName();
    void protocolGetHardwareVersion();
    void protocolGetFirmwareVersion();
    void protocolGetSerialNumber();
    void protocolGetBuildTimestamp();

    void checkTokenCount(size_t tokenCount, size_t expectedCount);

    void sendResponse(const String& tag, const String& data = {});
    void sendError(const String& error);

    auto toIndex(const String& s) -> size_t;

private:
    HostInterface m_hostInterface;
    PowerMonitor m_powerMonitor;
    SensorManager m_sensorManager;
};
