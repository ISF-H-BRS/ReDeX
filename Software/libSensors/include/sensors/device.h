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
  #ifdef ISF_BUILD_PROCESS
    #define ISF_EXPORT __declspec(dllexport)
  #else
    #define ISF_EXPORT __declspec(dllimport)
  #endif
#else
  #define ISF_EXPORT __attribute__((visibility("default")))
#endif

#include <memory>

namespace isf::Sensors {

class ISF_EXPORT Device
{
public:
    enum class SensorType
    {
        Unknown,
        pH_ORP,
        Temperature,
        None
    };

    static constexpr size_t SensorCount = 2;

    struct PowerValues
    {
        double voltage;
        double current;
        double temperature;
    };

    using Error = std::runtime_error;

public:
    Device(const std::string& port);
    ~Device();

    Device(const Device&) = delete;
    Device(Device&&) = delete;

    auto operator=(const Device&) = delete;
    auto operator=(Device&&) = delete;

    auto getSensorType(size_t index) const -> SensorType;
    auto getSensorValue(size_t index) const -> double;

    auto getPowerValues() const -> PowerValues;

    auto getHardwareVersion() const -> std::string;
    auto getFirmwareVersion() const -> std::string;
    auto getSerialNumber() const -> std::string;
    auto getBuildTimestamp() const -> unsigned long;

    static auto toPhValue(double voltage, double temperature = 25.0) -> double;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // End of namespace isf::Sensors
