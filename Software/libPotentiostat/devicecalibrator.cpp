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

#include <potentiostat/devicecalibrator.h>

#include <cassert>
#include <numeric>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Potentiostat;

// ---------------------------------------------------------------------------------------------- //

namespace  {
    constexpr std::chrono::seconds CalibrationDuration = 5s;
    constexpr auto CurrentRange = Device::CurrentRange::_10mA;
}

// ---------------------------------------------------------------------------------------------- //

class DeviceCalibrator::Private
{
public:
    Private(const char* port) : device(port) {}

    Device device;
    Listener* listener = nullptr;

    enum class Stage { Adc, Dac };
    Stage stage = Stage::Adc;

    double voltage = 0.0;
    double current = 0.0;

    int sampleCount = 0;

    Device::Calibration result = {};
};

// ---------------------------------------------------------------------------------------------- //

DeviceCalibrator::DeviceCalibrator(const char* port, Listener* listener)
    : d(std::make_unique<Private>(port))
{
    assert(listener != nullptr);
    d->device.addListener(this);
    d->listener = listener;
}

// ---------------------------------------------------------------------------------------------- //

DeviceCalibrator::~DeviceCalibrator() = default;

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::start()
{
    startAdc();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::abort()
{
    d->device.stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::startAdc()
{
    const Device::Setup setup = {
        Device::MeasurementType::OpenCircuit,
        CurrentRange,
        false,
        CalibrationDuration,
        Device::MinimumScanRate,
        0,
        0,
        0,
        Device::MinimumCycleCount
    };

    d->stage = Private::Stage::Adc;
    d->voltage = 0.0;
    d->current = 0.0;
    d->sampleCount = 0;

    d->result.voltageOffset = 0;
    d->result.currentOffset = 0;
    d->result.signalOffset = 0;

    d->device.setCalibration(d->result);
    d->device.startMeasurement(setup);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::startDac()
{
    const Device::Setup setup = {
        Device::MeasurementType::Electrolysis,
        CurrentRange,
        false,
        CalibrationDuration,
        Device::MinimumScanRate,
        0,
        0,
        0,
        Device::MinimumCycleCount
    };

    d->stage = Private::Stage::Dac;
    d->voltage = 0.0;
    d->current = 0.0;
    d->sampleCount = 0;

    d->device.setCalibration(d->result);
    d->device.startMeasurement(setup);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::onMeasurementComplete() noexcept
{
    if (d->stage == Private::Stage::Adc)
    {
        d->voltage /= d->sampleCount;
        d->current /= d->sampleCount;
        d->current *= Device::gainOf(CurrentRange);

        d->result.voltageOffset = Device::toAdcOffset(-d->voltage);
        d->result.currentOffset = Device::toAdcOffset(-d->current);

        startDac();
    }
    else
    {
        d->voltage /= d->sampleCount;
        d->result.signalOffset = Device::toDacOffset(-d->voltage);

        d->listener->onCalibrationComplete(d->result);
    }
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::onSamplesReceived(std::span<const double> voltages,
                                         std::span<const double> currents) noexcept
{
    static constexpr int TotalSampleCount =
            CalibrationDuration.count() * Device::SampleRate / Device::SampleBufferSize;

    d->voltage += std::accumulate(voltages.begin(), voltages.end(), 0.0) / voltages.size();
    d->current += std::accumulate(currents.begin(), currents.end(), 0.0) / currents.size();

    ++d->sampleCount;

    if (d->stage == Private::Stage::Adc)
        d->listener->onCalibrationProgress(50 * d->sampleCount / TotalSampleCount);
    else
        d->listener->onCalibrationProgress(50 + 50 * d->sampleCount / TotalSampleCount);
}

// ---------------------------------------------------------------------------------------------- //

void DeviceCalibrator::onError(const std::string& msg) noexcept
{
    d->listener->onError(msg);
}

// ---------------------------------------------------------------------------------------------- //
