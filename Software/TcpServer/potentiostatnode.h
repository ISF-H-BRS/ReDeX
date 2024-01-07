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

#include <potentiostat/device.h>

#include <exception>
#include <vector>

class PotentiostatSensor;

struct Voltammogram
{
    std::vector<double> voltage;
    std::vector<double> current;
};

class PotentiostatNode : public Node, public isf::Potentiostat::Device::Listener
{
    Q_OBJECT

public:
    static constexpr size_t InputCount = 1;

    using Device = isf::Potentiostat::Device;

public:
    PotentiostatNode(const QString& id, const QString& serialPort);

    auto alarmThresholds() const -> const Alarm::Thresholds& override;

    void startMeasurement() override;
    void stopMeasurement() override;

    void update() override;

private slots:
    void handlePowerValues(double voltage, double current, double temperature);

private:
    friend class PotentiostatSensor;
    void registerSensor(PotentiostatSensor* sensor, size_t input);
    void unregisterSensor(PotentiostatSensor* sensor);

    void setup(const Device::Setup& setup, const Device::Calibration& calibration);

private:
    void startNextMeasurement();
    void handleMeasurementComplete();

    void onMeasurementStarted() noexcept override;
    void onMeasurementStopped() noexcept override;
    void onMeasurementComplete() noexcept override;
    void onCurrentRangeChanged(Device::CurrentRange range) noexcept override;
    void onSamplesReceived(std::span<const double> voltages,
                           std::span<const double> currents) noexcept override;
    void onPowerValuesReceived(const Device::PowerValues& values) noexcept override;
    void onError(const std::string& msg) noexcept override;

private:
    Device m_device;
    Device::Setup m_setup = {};

    Voltammogram m_data;

    bool m_measurementStarted = false;
    bool m_measurementRunning = false;
    bool m_measurementComplete = false;

    std::exception_ptr m_exception;

    PotentiostatSensor* m_sensor = nullptr;
};
