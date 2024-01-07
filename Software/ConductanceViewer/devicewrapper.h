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

#include "device.h"
#include "sensorwrapper.h"

#include <QObject>
#include <QTimer>

#include <chrono>
#include <memory>

using namespace std::chrono_literals;

class DeviceWrapper : public QObject
{
    Q_OBJECT

public:
    DeviceWrapper(const DeviceInfo& info,
                  const std::array<DataBuffer*, Device::InputCount>& data);
    ~DeviceWrapper();

    DeviceWrapper(const DeviceWrapper&) = delete;
    auto operator=(const DeviceWrapper&) = delete;

    DeviceWrapper(DeviceWrapper&&) = delete;
    auto operator=(DeviceWrapper&&) = delete;

    void startCapture();
    void stopCapture();

    auto inputConnected(Device::Input input) const -> bool;

public slots:
    void setGain(Device::Input input, Device::Gain gain);

    void setupSignal(Device::Input input,
                     Device::Waveform waveform, unsigned int frequency, double amplitude);

    void setLeadResistance(Device::Input input, unsigned int milliohm);

signals:
    void dataUpdated();
    void error(const QString& msg);

    void analysisComplete(Device::Input input,
                          double voltage, double current, double admittance);

    void powerUpdated(const Device::PowerValues& values);

private slots:
    void handleDataAvailable(const Device::Data& data);
    void updatePowerValues();

private:
    void restartPowerTimer(std::chrono::milliseconds ms);

private:
    class Listener;
    std::unique_ptr<Listener> m_listener;

    Device m_device;

    std::array<std::unique_ptr<SensorWrapper>, Device::InputCount> m_sensors;

    QTimer m_powerTimer;
};
