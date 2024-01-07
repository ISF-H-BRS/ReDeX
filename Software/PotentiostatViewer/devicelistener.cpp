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

#include "devicelistener.h"

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onMeasurementStarted() noexcept
{
    QMetaObject::invokeMethod(this, "measurementStarted", Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onMeasurementStopped() noexcept
{
    QMetaObject::invokeMethod(this, "measurementStopped", Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onMeasurementComplete() noexcept
{
    QMetaObject::invokeMethod(this, "measurementComplete", Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onCurrentRangeChanged(Device::CurrentRange range) noexcept
{
    QMetaObject::invokeMethod(this, "currentRangeChanged", Qt::QueuedConnection,
                              Q_ARG(Device::CurrentRange, range));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onSamplesReceived(std::span<const double> voltages,
                                       std::span<const double> currents) noexcept
{
    SampleBuffer buffer;

    std::copy(voltages.begin(), voltages.end(), std::back_inserter(buffer.voltages));
    std::copy(currents.begin(), currents.end(), std::back_inserter(buffer.currents));

    QMetaObject::invokeMethod(this, "samplesReceived", Qt::QueuedConnection,
                              Q_ARG(DeviceListener::SampleBuffer, std::move(buffer)));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onPowerValuesReceived(const Device::PowerValues& values) noexcept
{
    QMetaObject::invokeMethod(this, "powerValuesReceived", Qt::QueuedConnection,
                              Q_ARG(Device::PowerValues, values));
}

// ---------------------------------------------------------------------------------------------- //

void DeviceListener::onError(const std::string& msg) noexcept
{
    QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                              Q_ARG(QString, msg.c_str()));
}

// ---------------------------------------------------------------------------------------------- //
