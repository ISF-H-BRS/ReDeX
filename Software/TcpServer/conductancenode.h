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

#include <conductance/analyzer.h>
#include <conductance/device.h>

#include <array>
#include <mutex>

class ConductanceSensor;

class ConductanceNode : public Node, public isf::Conductance::Device::Listener
{
    Q_OBJECT

public:
    using Analyzer = isf::Conductance::Analyzer;
    using Device = isf::Conductance::Device;
    using DeviceInfo = isf::Conductance::DeviceInfo;
    using DataBuffer = isf::Conductance::DataBuffer;

    static constexpr size_t InputCount = Device::InputCount;

public:
    ConductanceNode(const QString& id, const QString& serial);

    auto alarmThresholds() const -> const Alarm::Thresholds& override;

    void startMeasurement() override;
    void stopMeasurement() override;

    void update() override;

private:
    friend class ConductanceSensor;
    void registerSensor(ConductanceSensor* sensor, size_t input);
    void unregisterSensor(ConductanceSensor* sensor);

    void setupSignal(size_t input, Device::Waveform waveform,
                     unsigned int frequency, double amplitude);
    void setGain(size_t input, Device::Gain gain);

    void updateStatus();
    void updateMeasurement();

private:
    void onDataAvailable(const Device::Data& data) override;
    void onError(const std::string& msg) override;

private:
    std::unique_ptr<Device> m_device;

    std::array<DataBuffer, Device::InputCount> m_dataBuffers = {
        Device::Input::One, Device::Input::Two
    };

    std::mutex m_mutex;
    std::exception_ptr m_exception;

    std::array<ConductanceSensor*, InputCount> m_sensors = {};

    bool m_measurementStarted = false;
};
