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

#include "averagingbuffer.h"
#include "serialport.h"

#include <potentiostat/device.h>

#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <sstream>
#include <thread>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Potentiostat;

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr int OversampleFactor = 16;
    constexpr int FullScaleValue = OversampleFactor * 0xffff;

    constexpr double ReferenceVoltage = 4.096;
    constexpr double FullscaleVoltage = 8.192;

    constexpr double DacResolution = 0.0005;

    constexpr const char* LineBreak = "\r\n";
    constexpr size_t LineBreakSize = 2;

    auto split(const std::string& s, char delim) -> std::vector<std::string>
    {
        std::vector<std::string> result;

        std::istringstream stream(s);
        std::string token;

        while (std::getline(stream, token, delim))
            result.push_back(token);

        if (s.back() == delim)
            result.push_back("");

        return result;
    }

    template <typename T>
    auto to(const std::string& s) -> T
    {
        std::istringstream stream(s);

        T t = {};
        stream >> t;

        if (stream.fail())
            throw std::exception();

        return t;
    }

    template <typename T>
    auto toString(T value) -> std::string
    {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }
}

// ---------------------------------------------------------------------------------------------- //

class Device::Private
{
public:
    using StringList = std::vector<std::string>;

public:
    Private(const std::string& port);

    void work();

    void sendCommand(std::string command) const;

    void setCurrentRange(CurrentRange range);

    void parseData(std::span<const char> data);
    void processCurrentLine();

    void parseSample(const StringList& tokens);
    void parsePowerValues(const StringList& tokens);
    void parseError(const StringList& tokens);

    void updateCurrentRange(double current);

    template <typename Func, typename... Args>
    void notifyListeners(Func&& func, Args&&... args);

    static auto mapError(const std::string& error) -> std::string;

public:
    SerialPort serialPort;

    std::vector<Listener*> listeners;

    bool running = false;

    std::thread thread;

    std::string currentLine;

    std::vector<double> voltages;
    std::vector<double> currents;

    static constexpr size_t AveragingBufferSize = 10;
    AveragingBuffer<double, AveragingBufferSize> averageCurrent;

    bool autoRange = false;
    CurrentRange currentRange = CurrentRange::_10mA;
};

// ---------------------------------------------------------------------------------------------- //

class InvalidResponseError : public Device::Error
{
public:
    InvalidResponseError(const std::string& response)
        : Device::Error("Invalid response received from device: '" + response + "'") {}
};

// ---------------------------------------------------------------------------------------------- //

Device::Device(const std::string& port)
    : d(std::make_unique<Private>(port))
{
    d->running = true;
    d->thread = std::thread(&Device::Private::work, d.get());
}

// ---------------------------------------------------------------------------------------------- //

Device::~Device()
{
    assert(d->thread.joinable());

    d->running = false;
    d->thread.join();
}

// ---------------------------------------------------------------------------------------------- //

void Device::addListener(Listener* listener)
{
    assert(listener != nullptr);

    auto it = std::find(d->listeners.begin(), d->listeners.end(), listener);

    if (it == d->listeners.end())
        d->listeners.push_back(listener);
}

// ---------------------------------------------------------------------------------------------- //

void Device::removeListener(Listener* listener)
{
    auto it = std::find(d->listeners.begin(), d->listeners.end(), listener);

    if (it != d->listeners.end())
        d->listeners.erase(it);
}

// ---------------------------------------------------------------------------------------------- //

void Device::setCalibration(const Calibration& calibration)
{
    const std::string command = "<SET_CALIBRATION> " +
                                std::to_string(calibration.voltageOffset) + ";" +
                                std::to_string(calibration.currentOffset) + ";" +
                                std::to_string(calibration.signalOffset);
    d->sendCommand(command);
}

// ---------------------------------------------------------------------------------------------- //

void Device::startMeasurement(const Setup& setup)
{
    d->voltages.clear();
    d->currents.clear();

    d->autoRange = setup.autoRange;
    d->currentRange = setup.currentRange;

    const std::string command = "<START_MEASUREMENT> " +
                                std::to_string(indexOf(setup.measurementType)) + ";" +
                                std::to_string(indexOf(setup.currentRange)) + ";" +
                                std::to_string(setup.duration.count()) + ";" +
                                std::to_string(setup.scanRate) + ";" +
                                std::to_string(setup.vertex0) + ";" +
                                std::to_string(setup.vertex1) + ";" +
                                std::to_string(setup.vertex2) + ";" +
                                std::to_string(setup.cycleCount);
    d->sendCommand(command);
}

// ---------------------------------------------------------------------------------------------- //

void Device::stopMeasurement()
{
    d->sendCommand("<STOP_MEASUREMENT>");
}

// ---------------------------------------------------------------------------------------------- //

void Device::setCurrentRange(CurrentRange range)
{
    d->setCurrentRange(range);
}

// ---------------------------------------------------------------------------------------------- //

void Device::requestPowerValues()
{
    d->sendCommand("<GET_POWER_VALUES>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::valueOf(CurrentRange range) -> double
{
    static constexpr std::array<double, CurrentRangeCount> Values = {
        200.0e-9, 1.0e-6, 100.0e-6, 10.0e-3
    };

    return Values.at(indexOf(range));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::gainOf(CurrentRange range) -> double
{
    static constexpr std::array<double, CurrentRangeCount> Gains = {
        20.0e6, 4.0e6, 40.0e3, 400.0
    };

    return Gains.at(indexOf(range));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toString(CurrentRange range) -> std::string
{
    static constexpr std::array<const char*, CurrentRangeCount> Strings = {
        "200 nA", "1 µA", "100 µA", "10 mA"
    };

    return Strings.at(indexOf(range));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toAdcOffset(double voltage) -> int
{
    return static_cast<int>(std::round(voltage * FullScaleValue / FullscaleVoltage));
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toDacOffset(double voltage) -> int
{
    return static_cast<int>(std::round(voltage / DacResolution));
}

// ---------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------- //

Device::Private::Private(const std::string& port)
    : serialPort(port) {}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::work()
{
    static constexpr std::chrono::milliseconds ReadInterval = 50ms;

    while (running)
    {
        try {
            const bool dataAvailable = serialPort.getNumberOfBytesAvailable() > 0;

            if (dataAvailable)
                parseData(serialPort.readAllData());
        }
        catch (const std::exception& e) {
            notifyListeners(&Listener::onError, e.what());
        }

        std::this_thread::sleep_for(ReadInterval);
    }
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::sendCommand(std::string command) const
{
    command += "\r\n";
    serialPort.sendData(command);
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::setCurrentRange(CurrentRange range)
{
    currentRange = range;
    sendCommand("<SET_GAIN> " + std::to_string(indexOf(range)));
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::parseData(std::span<const char> data)
{
    for (char c : data)
    {
        currentLine += c;

        if (currentLine.ends_with(LineBreak))
            processCurrentLine();
    }
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::processCurrentLine()
{
    assert(currentLine.ends_with(LineBreak));

    const std::string response(currentLine.begin(), currentLine.end() - LineBreakSize);
    currentLine.clear();

    const std::vector<std::string> tokens = split(response, ' ');

    if (tokens.empty())
        return;

    try {
        const std::string& tag = tokens[0];

        if (tag == "<S>")
            parseSample(tokens);
        else if (tag == "<P>")
            parsePowerValues(tokens);
        else if (tag == "<MEASUREMENT_STARTED>")
            notifyListeners(&Listener::onMeasurementStarted);
        else if (tag == "<MEASUREMENT_STOPPED>")
            notifyListeners(&Listener::onMeasurementStopped);
        else if (tag == "<MEASUREMENT_COMPLETE>")
        {
            if (!voltages.empty())
            {
                assert(voltages.size() == currents.size());
                notifyListeners(&Listener::onSamplesReceived, voltages, currents);
            }

            notifyListeners(&Listener::onMeasurementComplete);
        }
        else if (tag == "<GAIN_SET>")
            notifyListeners(&Listener::onCurrentRangeChanged, currentRange);
        else if (tag == "<ERROR>")
            parseError(tokens);
        else
            notifyListeners(&Listener::onError, "Unknown tag " + tag + " received from device.");
    }
    catch (const std::exception& e) {
        notifyListeners(&Listener::onError, e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::parseSample(const StringList& tokens)
{
    static constexpr size_t ExpectedTokenCount = 2;
    static constexpr size_t ExpectedValueCount = 3;

    const auto toVoltage = [](auto value) -> double {
        return value * FullscaleVoltage / FullScaleValue - ReferenceVoltage;
    };

    if (tokens.size() != ExpectedTokenCount)
        throw Error("Invalid number of sample tokens received.");

    StringList values = split(tokens[1], ';');

    if (values.size() != ExpectedValueCount)
        throw Error("Invalid number of sample values received.");

    try {
        const auto gain    = to<int>(values[0]);
        const auto voltage = to<int>(values[1]);
        const auto current = to<int>(values[2]);

        const double gainValue = gainOf(toCurrentRange(gain));

        const double realVoltage = toVoltage(voltage);
        const double realCurrent = toVoltage(current) / gainValue;

        voltages.push_back(realVoltage);
        currents.push_back(realCurrent);

        if (autoRange)
            updateCurrentRange(realCurrent);

        if (voltages.size() >= SampleBufferSize)
        {
            assert(voltages.size() == currents.size());
            notifyListeners(&Listener::onSamplesReceived, voltages, currents);

            voltages.clear();
            currents.clear();
        }
    }
    catch (...) {
        throw Error("Invalid sample value received.");
    }
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::parsePowerValues(const StringList& tokens)
{
    static constexpr size_t ExpectedTokenCount = 2;
    static constexpr size_t ExpectedValueCount = 3;

    if (tokens.size() != ExpectedTokenCount)
        throw Error("Invalid number of power-value tokens received.");

    const StringList values = split(tokens[1], ';');

    if (values.size() != ExpectedValueCount)
        throw Error("Invalid number of power values received.");

    try {
        const PowerValues power = {
            to<double>(values[0]),
            to<double>(values[1]),
            to<double>(values[2])
        };

        notifyListeners(&Listener::onPowerValuesReceived, power);
    }
    catch (...) {
        throw Error("Invalid power value received.");
    }
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::parseError(const StringList& tokens)
{
    static constexpr size_t MinimumTokenCount = 1;

    std::string error;

    if (tokens.size() > MinimumTokenCount)
        error = mapError(tokens[1]);

    notifyListeners(&Listener::onError, error);
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::updateCurrentRange(double current)
{
    averageCurrent.addSample(current);

    const double threshold = 0.8 * valueOf(currentRange);
    const double avg = std::fabs(averageCurrent.getValue());
    const auto index = indexOf(currentRange);

    if (avg > threshold && index < CurrentRangeCount - 1)
    {
        averageCurrent.clear();
        setCurrentRange(toCurrentRange(index + 1));
    }
}

// ---------------------------------------------------------------------------------------------- //

template <typename Func, typename... Args>
void Device::Private::notifyListeners(Func&& func, Args&&... args)
{
    for (auto listener : listeners)
        std::invoke(std::forward<Func>(func), listener, std::forward<Args>(args)...);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::mapError(const std::string& error) -> std::string
{
    if (error == "DATA_OVERFLOW")
        return "Data overflow.";

    if (error == "UNKNOWN_COMMAND")
        return "Unknown command.";

    if (error == "MISSING_ARGUMENT")
        return "Missing argument.";

    if (error == "INVALID_ARGUMENT")
        return "Invalid argument.";

    if (error == "INVALID_ADDRESS")
        return "Invalid address.";

    if (error == "INVALID_LENGTH")
        return "Invalid length.";

    if (error == "ERASE_FAILED")
        return "Erase failed.";

    if (error == "WRITE_FAILED")
        return "Write failed.";

    if (error == "DATA_MISMATCH")
        return "Data mismatch.";

    return "Unknown error code received: " + error;
}

// ---------------------------------------------------------------------------------------------- //
