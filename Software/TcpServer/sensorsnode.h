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

#include "node.h"

#include <sensors/device.h>

#include <array>

class SensorsSensor;

class SensorsNode : public Node
{
public:
    static constexpr size_t InputCount = 2;

    using Device = isf::Sensors::Device;
    using SensorType = Device::SensorType;

public:
    SensorsNode(const QString& id, const QString& serialPort);

    auto getSensorType(size_t index) const -> SensorType;

    auto alarmThresholds() const -> const Alarm::Thresholds& override;

    void startMeasurement() override;
    void stopMeasurement() override;

    void update() override;

private:
    friend class SensorsSensor;
    void registerSensor(SensorsSensor* sensor, size_t input);
    void unregisterSensor(SensorsSensor* sensor);

    void updateStatus();
    void updateMeasurement();

private:
    Device m_device;
    std::array<SensorsSensor*, InputCount> m_sensors = {};

    bool m_measurementStarted = false;
};
