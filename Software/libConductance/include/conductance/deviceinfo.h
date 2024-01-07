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

#include <conductance/global.h>

#include <memory>
#include <string>
#include <vector>

ISF_CONDUCTANCE_BEGIN_NAMESPACE();

class ISF_EXPORT DeviceInfo
{
public:
    using NoDeviceError = std::runtime_error;

public:
    auto serialNumber() const -> const std::string&;
    auto productName() const -> const std::string&;

    auto busNumber() const -> uint8_t;
    auto portNumber() const -> uint8_t;

    static auto getAvailableDevices() -> std::vector<DeviceInfo>;
    static auto getFirstDevice() -> DeviceInfo;

private:
    DeviceInfo(const std::string& product, const std::string& serial, uint8_t bus, uint8_t port);

private:
    std::string m_productName;
    std::string m_serialNumber;

    uint8_t m_busNumber;
    uint8_t m_portNumber;
};

ISF_CONDUCTANCE_END_NAMESPACE();
