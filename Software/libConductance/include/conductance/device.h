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

#include <conductance/deviceinfo.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

ISF_CONDUCTANCE_BEGIN_NAMESPACE();

class ISF_EXPORT Device
{
public:
    static constexpr const char* ProductName = "Conductance Board";

    static constexpr uint16_t VendorId = 0x0483;
    static constexpr uint16_t ProductId = 0x5750;

    enum class Input
    {
        One,
        Two
    };

    static constexpr size_t InputCount = 2;

    template <typename T = size_t>
    static constexpr auto indexOf(Input input) { return static_cast<T>(input); }

    template <typename T = size_t>
    static constexpr auto toInput(T index) { return static_cast<Input>(index); }

    enum class Channel
    {
        Voltage,
        Current
    };

    static constexpr size_t ChannelCount = 2;

    template <typename T = size_t>
    static constexpr auto indexOf(Channel channel) { return static_cast<T>(channel); }

    template <typename T = size_t>
    static constexpr auto toChannel(T index) { return static_cast<Channel>(index); }

    static constexpr auto VoltageChannelIndex = static_cast<size_t>(Channel::Voltage);
    static constexpr auto CurrentChannelIndex = static_cast<size_t>(Channel::Current);

    enum class Waveform
    {
        None     = 0,
        Sine     = 1,
        Square   = 2,
        Triangle = 3,
        Sawtooth = 4
    };

    static constexpr size_t WaveformCount = 5;

    template <typename T = size_t>
    static constexpr auto indexOf(Waveform waveform) { return static_cast<T>(waveform); }

    template <typename T = size_t>
    static constexpr auto toWaveform(T index) { return static_cast<Waveform>(index); }

    enum class Gain
    {
        _100,
        _1k,
        _10k,
        _100k
    };

    static constexpr size_t GainCount = 4;

    template <typename T = size_t>
    static constexpr auto indexOf(Gain gain) { return static_cast<T>(gain); }

    template <typename T = size_t>
    static constexpr auto toGain(T index) { return static_cast<Gain>(index); }

    static constexpr size_t SamplesPerTransfer = 500;

    static constexpr unsigned int SampleRate = 10000;

    static constexpr unsigned int MinimumFrequency = 1;
    static constexpr unsigned int MaximumFrequency = 1000;

    static constexpr double MinimumAmplitude = 0.0;
    static constexpr double MaximumAmplitude = 1.5;

    using PerChannelData = std::array<double, SamplesPerTransfer>;
    using PerInputData = std::array<PerChannelData, ChannelCount>;
    using Data = std::array<PerInputData, InputCount>;

    struct PowerValues
    {
        double voltage;
        double current;
        double temperature;
    };

public:
    class Listener
    {
    public:
        virtual void onDataAvailable(const Data&) {};
        virtual void onError(const std::string&) {};
    };

public:
    Device(const DeviceInfo& info);
    Device();
    ~Device();

    Device(const Device&) = delete;
    auto operator=(const Device&) = delete;

    Device(Device&&) = delete;
    auto operator=(Device&&) = delete;

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    auto getInputsConnected() const -> std::array<bool, InputCount>;

    void setupSignal(Input input, Waveform waveform, unsigned int frequency, double amplitude);
    void setGain(Input input, Gain gain);

    void startCapture();
    void stopCapture();

    auto getPowerValues() const -> PowerValues;

    static auto toString(Waveform waveform) -> const char*;

    static auto toString(Gain gain) -> const char*;
    static auto toDouble(Gain gain) -> double;

private:
    void work();

private:
    class Private;
    std::unique_ptr<Private> d;
};

ISF_CONDUCTANCE_END_NAMESPACE();
