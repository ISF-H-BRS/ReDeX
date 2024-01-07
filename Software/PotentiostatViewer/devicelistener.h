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

#include <QObject>

class DeviceListener : public QObject, public Device::Listener
{
    Q_OBJECT

public:
    struct SampleBuffer
    {
        std::vector<double> voltages;
        std::vector<double> currents;
    };

signals:
    void measurementStarted();
    void measurementStopped();
    void measurementComplete();
    void currentRangeChanged(Device::CurrentRange range);
    void samplesReceived(const DeviceListener::SampleBuffer& samples);
    void powerValuesReceived(const Device::PowerValues& values);
    void error(const QString& msg);

private:
    void onMeasurementStarted() noexcept override;
    void onMeasurementStopped() noexcept override;
    void onMeasurementComplete() noexcept override;
    void onCurrentRangeChanged(Device::CurrentRange range) noexcept override;
    void onSamplesReceived(std::span<const double> voltages,
                           std::span<const double> currents) noexcept override;
    void onPowerValuesReceived(const Device::PowerValues& values) noexcept override;
    void onError(const std::string& msg) noexcept override;
};
