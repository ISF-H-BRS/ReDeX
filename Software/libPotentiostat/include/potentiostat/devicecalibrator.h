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

#include <potentiostat/device.h>

ISF_POTENTIOSTAT_BEGIN_NAMESPACE();

class ISF_EXPORT DeviceCalibrator : public Device::Listener
{
public:
    class Listener
    {
        friend class DeviceCalibrator;

    protected:
        ~Listener() = default;

        virtual void onCalibrationProgress(int /*percent*/) noexcept {}
        virtual void onCalibrationComplete(const Device::Calibration& /*results*/) noexcept {}
        virtual void onError(const std::string& /*msg*/) noexcept {}
    };

public:
    DeviceCalibrator(const char* port, Listener* listener);
    ~DeviceCalibrator();

    DeviceCalibrator(const DeviceCalibrator&) = delete;
    DeviceCalibrator(DeviceCalibrator&&) = delete;

    auto operator=(const DeviceCalibrator&) = delete;
    auto operator=(DeviceCalibrator&&) = delete;

    void start();
    void abort();

private:
    void startAdc();
    void startDac();

    void onMeasurementComplete() noexcept override;
    void onSamplesReceived(std::span<const double> voltages,
                           std::span<const double> currents) noexcept override;
    void onError(const std::string& msg) noexcept override;

private:
    class Private;
    std::unique_ptr<Private> d;
};

ISF_POTENTIOSTAT_END_NAMESPACE();
