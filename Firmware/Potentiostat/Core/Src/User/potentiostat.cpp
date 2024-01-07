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

#include "config.h"
#include "delay.h"
#include "potentiostat.h"

#include "usbd_desc.h"

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr char TokenSeparator = ' ';
}

// ---------------------------------------------------------------------------------------------- //

class MissingArgumentError : public std::exception
{
public:
    auto what() const noexcept -> const char* { return "MISSING_ARGUMENT"; }
};

// ---------------------------------------------------------------------------------------------- //

class InvalidArgumentError : public std::exception
{
public:
    auto what() const noexcept -> const char* { return "INVALID_ARGUMENT"; }
};

// ---------------------------------------------------------------------------------------------- //

Potentiostat::Potentiostat()
    : m_hostInterface(this),
      m_signalGenerator(this),
      m_signalReader(this),
      m_blinker(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin)
{
}

// ---------------------------------------------------------------------------------------------- //

Potentiostat::~Potentiostat()
{
    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_SET);
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::exec()
{
    HAL_GPIO_WritePin(PWR_ENABLE_GPIO_Port, PWR_ENABLE_Pin, GPIO_PIN_SET); // Active high
    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_RESET); // Active low

    while (true)
    {
        m_hostInterface.update();
        m_powerMonitor.update();

        if (m_measurementRunning)
        {
            m_signalGenerator.update();
            m_signalReader.update();
            m_blinker.update();
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::onHostDataReceived(const String& data)
{
    const size_t tokenCount = data.getTokenCount(TokenSeparator);

    if (tokenCount < 1)
        return;

    const String tag = data.getToken(TokenSeparator, 0);

    if (tag == "<START_MEASUREMENT>")
        protocolStartMeasurement(data, tokenCount);
    else if (tag == "<STOP_MEASUREMENT>")
        protocolStopMeasurement();
    else if (tag == "<GET_POWER_VALUES>")
        protocolGetPowerValues();
    else if (tag == "<SET_GAIN>")
        protocolSetGain(data, tokenCount);
    else if (tag == "<SET_CALIBRATION>")
        protocolSetCalibration(data, tokenCount);
    else
        sendError("UNKNOWN_COMMAND");
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::onHostDataOverflow()
{
    sendError("DATA_OVERFLOW");
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::onSamplesAvailable(std::span<const Measurement::Sample> samples)
{
    ASSERT(samples.size() <= m_sampleStrings.size());

    for (size_t i = 0; i < samples.size(); ++i)
    {
        const Measurement::Sample& sample = samples[i];
        m_sampleStrings[i].format("<S> %d;%d;%d", sample.gain, sample.voltage, sample.current);
    }

    if (!m_hostInterface.sendData(m_sampleStrings))
        stopMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::onSignalGenerationComplete()
{
    stopMeasurement();
    m_hostInterface.sendData("<MEASUREMENT_COMPLETE>");
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::protocolGetPowerValues()
{
    const double voltage     = m_powerMonitor.getVoltage();
    const double current     = m_powerMonitor.getCurrent();
    const double temperature = m_powerMonitor.getTemperature();

    sendResponse("<P>", String::makeFormat("%f;%f;%f", voltage, current, temperature));
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::protocolSetCalibration(const String& data, size_t tokenCount)
{
    static constexpr size_t ExpectedTokenCount = 2;
    static constexpr size_t ExpectedValueCount = 3;

    static constexpr char ValueSeparator = ';';

    if (tokenCount < ExpectedTokenCount)
        return sendError("MISSING_ARGUMENT");

    const String values = data.getToken(TokenSeparator, 1);
    const size_t valueCount = values.getTokenCount(ValueSeparator);

    if (valueCount != ExpectedValueCount)
        return sendError("INVALID_LENGTH");

    std::array<String, ExpectedValueCount> valueTokens;
    values.getAllTokens(ValueSeparator, valueTokens);

    const auto voltage = valueTokens[0].toInt();
    const auto current = valueTokens[1].toInt();
    const auto signal = valueTokens[2].toInt();

    const bool valid = voltage >= SignalReader::MinimumCalibrationOffset &&
                       voltage <= SignalReader::MaximumCalibrationOffset &&
                       current >= SignalReader::MinimumCalibrationOffset &&
                       current <= SignalReader::MaximumCalibrationOffset &&
                       signal >= SignalGenerator::MinimumCalibrationOffset &&
                       signal <= SignalGenerator::MaximumCalibrationOffset;
    if (!valid)
        return sendError("INVALID_ARGUMENT");

    m_signalReader.setCalibrationOffsets(voltage, current);
    m_signalGenerator.setCalibrationOffset(signal);
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::protocolStartMeasurement(const String& data, size_t tokenCount)
{
    static constexpr size_t ExpectedTokenCount = 2;
    static constexpr size_t ExpectedValueCount = 8;

    static constexpr char ValueSeparator = ';';

    if (m_measurementRunning)
        return sendError("DEVICE_BUSY");

    if (tokenCount < ExpectedTokenCount)
        return sendError("MISSING_ARGUMENT");

    const String values = data.getToken(TokenSeparator, 1);
    const size_t valueCount = values.getTokenCount(ValueSeparator);

    if (valueCount != ExpectedValueCount)
        return sendError("INVALID_LENGTH");

    std::array<String, ExpectedValueCount> valueTokens;
    values.getAllTokens(ValueSeparator, valueTokens);

    const auto type = Measurement::toType(valueTokens[0].toUInt());
    const auto gain = Measurement::toGain(valueTokens[1].toUInt());
    const auto duration = std::chrono::seconds(valueTokens[2].toUInt());
    const auto rate = std::clamp(valueTokens[3].toInt(),
                                 Measurement::MinimumScanRate, Measurement::MaximumScanRate);
    const auto vertex0 = valueTokens[4].toInt();
    const auto vertex1 = valueTokens[5].toInt();
    const auto vertex2 = valueTokens[6].toInt();
    const auto cycles = valueTokens[7].toInt();

    Measurement::Setup setup = {
            type,
            gain,
            duration,
            rate,
            vertex0,
            vertex1,
            vertex2,
            cycles
    };

    if (!getSetupValid(setup))
        return sendError("INVALID_ARGUMENT");

    setGain(gain);

    m_signalGenerator.prepare(setup);
    m_hostInterface.sendData("<MEASUREMENT_STARTED>");

    HAL_Delay(400);

    if (type != Measurement::Type::OpenCircuit)
        HAL_GPIO_WritePin(OUTPUT_ENABLE_GPIO_Port, OUTPUT_ENABLE_Pin, GPIO_PIN_RESET); // Active low

    HAL_Delay(100);

    startMeasurement();
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::protocolStopMeasurement()
{
    stopMeasurement();
    m_hostInterface.sendData("<MEASUREMENT_STOPPED>");
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::protocolSetGain(const String& data, size_t tokenCount)
{
    static constexpr size_t ExpectedTokenCount = 2;

    if (tokenCount < ExpectedTokenCount)
        return sendError("MISSING_ARGUMENT");

    const auto gain = data.getToken(TokenSeparator, 1).toUInt();

    if (gain >= Measurement::GainCount)
        return sendError("INVALID_ARGUMENT");

    setGain(Measurement::toGain(gain));

    m_hostInterface.sendData("<GAIN_SET>");
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::checkTokenCount(size_t tokenCount, size_t expectedCount)
{
    if (tokenCount < expectedCount)
        throw MissingArgumentError();
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::sendResponse(const String& tag, const String& data)
{
    auto response = tag;

    if (!data.empty())
        response += " " + data;

    m_hostInterface.sendData(response);
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::sendError(const String& error)
{
    m_hostInterface.sendData("<ERROR> " + error);
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::startMeasurement()
{
    if (m_measurementRunning)
        return;

    m_measurementRunning = true;

    m_blinker.start();
    m_signalReader.start(m_currentGain);
    m_signalGenerator.start();
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::stopMeasurement()
{
    if (!m_measurementRunning)
        return;

    m_signalGenerator.stop();
    m_signalReader.stop();
    m_blinker.stop();

    HAL_GPIO_WritePin(OUTPUT_ENABLE_GPIO_Port, OUTPUT_ENABLE_Pin, GPIO_PIN_SET); // Active low

    m_measurementRunning = false;
}

// ---------------------------------------------------------------------------------------------- //

void Potentiostat::setGain(Measurement::Gain gain)
{
    if (m_measurementRunning)
    {
        m_signalGenerator.pause();
        m_signalReader.stop();
    }

    m_gainMux.setGain(gain);
    m_currentGain = gain;

    Delay::wait(2ms); // Wait for filters to settle

    if (m_measurementRunning)
    {
        m_signalReader.start(m_currentGain);
        m_signalGenerator.unpause();
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Potentiostat::getSetupValid(const Measurement::Setup& setup) const -> bool
{
    static const auto inRange = [](auto value, auto min, auto max) {
        return value >= min && value <= max;
    };

    if (!inRange(setup.duration, Measurement::MinimumDuration, Measurement::MaximumDuration))
        return false;

    if (!inRange(setup.scanRate, Measurement::MinimumScanRate, Measurement::MaximumScanRate))
        return false;

    for (int value : { setup.vertex0, setup.vertex1, setup.vertex2 })
    {
        if (!inRange(value, Measurement::MinimumPotential, Measurement::MaximumPotential))
            return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------- //
