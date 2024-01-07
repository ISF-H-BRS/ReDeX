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

#include <potentiostat/global.h>

#include <chrono>
#include <memory>
#include <span>
#include <string>

using namespace std::chrono_literals;

ISF_POTENTIOSTAT_BEGIN_NAMESPACE();

class ISF_EXPORT Device
{
public:
    enum class MeasurementType
    {
        OpenCircuit,
        Electrolysis,
        LinearSweep,
        CyclicVoltammetry
    };

    static constexpr size_t MeasurementTypeCount = 4;

    template <typename T = size_t>
    static constexpr auto indexOf(MeasurementType type) { return static_cast<T>(type); }

    template <typename T>
    static constexpr auto toMeasurementType(T index) { return static_cast<MeasurementType>(index); }

    enum class CurrentRange
    {
        _200nA,
        _1uA,
        _100uA,
        _10mA
    };

    static constexpr size_t CurrentRangeCount = 4;

    template <typename T = size_t>
    static constexpr auto indexOf(CurrentRange range) { return static_cast<T>(range); }

    template <typename T = size_t>
    static constexpr auto toCurrentRange(T index) { return static_cast<CurrentRange>(index); }

    static constexpr std::chrono::seconds MinimumDuration = 1s;
    static constexpr std::chrono::seconds MaximumDuration = 600s;

    static constexpr int MinimumScanRate = 1;
    static constexpr int MaximumScanRate = 100;

    static constexpr int MinimumPotential = -2000;
    static constexpr int MaximumPotential = +2000;

    static constexpr int MinimumCycleCount = 1;
    static constexpr int MaximumCycleCount = 10;

    struct Setup
    {
        MeasurementType measurementType = MeasurementType::OpenCircuit;
        CurrentRange currentRange = CurrentRange::_10mA;
        bool autoRange = false;
        std::chrono::seconds duration = MinimumDuration;
        int scanRate = MaximumScanRate;
        int vertex0 = 0;
        int vertex1 = 0;
        int vertex2 = 0;
        int cycleCount = MinimumCycleCount;
    };

    static constexpr int SampleRate = 1000;
    static constexpr double SampleTime = 1.0 / SampleRate;

    static constexpr size_t SampleBufferSize = 100;

    static constexpr int MinimumCalibrationOffset = -100;
    static constexpr int MaximumCalibrationOffset = +100;

    struct Calibration
    {
        int voltageOffset = 0;
        int currentOffset = 0;
        int signalOffset = 0;
    };

    struct PowerValues
    {
        double voltage;
        double current;
        double temperature;
    };

    class Listener
    {
        friend class Device;

    protected:
        ~Listener() = default;

        virtual void onMeasurementStarted() noexcept {}
        virtual void onMeasurementStopped() noexcept {}
        virtual void onMeasurementComplete() noexcept {}

        virtual void onCurrentRangeChanged(CurrentRange /*range*/) noexcept {}

        virtual void onSamplesReceived(std::span<const double> /*voltages*/,
                                       std::span<const double> /*currents*/) noexcept {}

        virtual void onPowerValuesReceived(const PowerValues& /*values*/) noexcept {}

        virtual void onError(const std::string& /*msg*/) noexcept {}
    };

    using Error = std::runtime_error;

public:
    Device(const std::string& port);
    ~Device();

    Device(const Device&) = delete;
    Device(Device&&) = delete;

    auto operator=(const Device&) = delete;
    auto operator=(Device&&) = delete;

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    void setCalibration(const Calibration& calibration);

    void startMeasurement(const Setup& setup);
    void stopMeasurement();

    void setCurrentRange(CurrentRange range);

    void requestPowerValues();

    static auto valueOf(CurrentRange range) -> double;
    static auto gainOf(CurrentRange range) -> double;

    static auto toString(CurrentRange range) -> std::string;

    static auto toAdcOffset(double voltage) -> int;
    static auto toDacOffset(double voltage) -> int;

private:
    class Private;
    std::unique_ptr<Private> d;
};

ISF_POTENTIOSTAT_END_NAMESPACE();
