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

#include "tcpclient.h"

#include <cassert>
#include <sstream>

// ---------------------------------------------------------------------------------------------- //

using namespace redex;

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr uint16_t TcpPort = 5432;

    constexpr const char* LineTerminator = "\r\n";
    constexpr size_t LineTerminatorLength = 2;

    constexpr char TokenSeparator = 0x1f; // ASCII unit separator
    constexpr char ValueSeparator = ';';

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
            throw std::runtime_error("Invalid value " + s + " received.");

        return t;
    }

    template <>
    auto to(const std::string& s) -> AlarmType
    {
        if (s == "OVERVOLTAGE")
            return AlarmType::Overvoltage;

        if (s == "UNDERVOLTAGE")
            return AlarmType::Undervoltage;

        if (s == "OVERCURRENT")
            return AlarmType::Overcurrent;

        if (s == "OVERHEAT")
            return AlarmType::Overheat;

        throw std::runtime_error("Invalid alarm type " + s + " received.");
    }

    template <>
    auto to(const std::string& s) -> AlarmSeverity
    {
        if (s == "WARNING")
            return AlarmSeverity::Warning;

        if (s == "CRITICAL")
            return AlarmSeverity::Critical;

        throw std::runtime_error("Invalid alarm severity " + s + " received.");
    }

    auto toDouble(const std::string& s) -> double
    {
        if (s == "nan")
            return std::numeric_limits<double>::quiet_NaN();

        return to<double>(s);
    }
}
// ---------------------------------------------------------------------------------------------- //

TcpClient::TcpClient(const std::string& host, Listener* listener)
    : m_socket(TcpAddress::fromHostName(host), TcpPort),
      m_listener(listener)
{
    assert(listener != nullptr);

    waitForPreamble();

    m_running = true;
    m_thread = std::thread(&TcpClient::work, this);
}

// ---------------------------------------------------------------------------------------------- //

TcpClient::~TcpClient()
{
    assert(m_thread.joinable());

    try {
        stopMeasurement();
    }
    catch (...) {
    }

    m_running = false;
    m_thread.join();
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::requestNodeInfo()
{
    sendData("<GET_NODE_INFO>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::requestTestpointInfo()
{
    sendData("<GET_TESTPOINT_INFO>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::startPowerMonitor()
{
    sendData("<START_POWER_MONITOR>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::stopPowerMonitor()
{
    sendData("<STOP_POWER_MONITOR>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::startMeasurement()
{
    sendData("<START_MEASUREMENT>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::stopMeasurement()
{
    sendData("<STOP_MEASUREMENT>\r\n");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::waitForPreamble()
{
    static constexpr std::chrono::milliseconds Timeout = 500ms;
    static constexpr size_t MaximumStringLength = 255;

    std::string string;

    while (!string.ends_with(LineTerminator))
    {
        if (!m_socket.waitForDataAvailable(Timeout))
            throw Error("Server failed to respond.");

        const std::vector<char> data = m_socket.readAllData();
        string += std::string(data.begin(), data.end());

        if (string.length() > MaximumStringLength)
            throw Error("Garbage received from server.");
    }

    string.erase(string.size() - LineTerminatorLength);

    const std::vector<std::string> tokens = split(string, TokenSeparator);

    if (tokens.front() == "<ERROR>")
        throw Error(tokens.back());

    if (tokens.front() != "<WELCOME>")
        throw Error("Invalid preamble received from server.");
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::work()
{
    while (m_running)
    {
        if (m_socket.waitForDataAvailable())
            processData(m_socket.readAllData());
    }
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::processData(std::span<const char> data)
{
    for (char byte : data)
    {
        m_currentData += byte;

        if (m_currentData.ends_with(LineTerminator))
        {
            m_currentData.erase(m_currentData.size() - LineTerminatorLength);

            const std::vector<std::string> tokens = split(m_currentData, TokenSeparator);
            m_currentData.clear();

            try {
                if (tokens.front() == "<NODE_INFO>")
                    parseNodeInfo(tokens);
                else if (tokens.front() == "<TESTPOINT_INFO>")
                    parseTestpointInfo(tokens);
                else if (tokens.front() == "<HUB_ALARM>")
                    parseHubAlarm(tokens);
                else if (tokens.front() == "<NODE_ALARM>")
                    parseNodeAlarm(tokens);
                else if (tokens.front() == "<HUB_STATUS>")
                    parseHubStatus(tokens);
                else if (tokens.front() == "<NODE_STATUS>")
                    parseNodeStatus(tokens);
                else if (tokens.front() == "<STATUS>")
                    parseStatus(tokens);
                else if (tokens.front() == "<ERROR>")
                    parseError(tokens);
                else if (tokens.front() == "<VOLTAMMOGRAM>")
                    parseVoltammogram(tokens);
                else
                    parseSensorData(tokens);
            }
            catch (const std::exception& e) {
                m_listener->onError(e.what());
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

auto TcpClient::parseSensorInfo(std::span<const std::string> values) -> SensorInfo
{
    static constexpr size_t RequiredValueCount = 3; // id, nodeId, input

    if (values.size() != RequiredValueCount)
        throw Error("Invalid number of sensor-info values received.");

    SensorInfo info =  {};
    info.sensorId = values[0];
    info.nodeId = values[1];
    info.input = values[2].empty() ? 0 : to<size_t>(values[2]);

    return info;
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseNodeInfo(std::span<const std::string> tokens)
{
    static constexpr size_t MinimumTokenCount = 2; // tag, count
    static constexpr size_t RequiredValueCount = 2; // id, type

    if (tokens.size() < MinimumTokenCount)
        throw Error("Insufficient number of node-info tokens received.");

    assert(tokens[0] == "<NODE_INFO>");

    const auto count = to<size_t>(tokens[1]);

    if (count != tokens.size() - MinimumTokenCount)
        throw Error("Missing node-info tokens in received data.");

    std::vector<NodeInfo> infos(count);

    for (size_t i = 0; i < count; ++i)
    {
        const std::string& token = tokens[2 + i];
        const std::vector<std::string> values = split(token, ValueSeparator);

        if (values.size() != RequiredValueCount)
            throw Error("Invalid number of node-info values received.");

        const std::string& id = values[0];

        if (id.empty())
            throw Error("Empty node ID received.");

        const std::string& type = values[1];

        if (type.empty())
            throw Error("Empty node type received.");

        infos[i].id = id;
        infos[i].type = type;
    }

    m_listener->onNodeInfoReceived(infos);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseTestpointInfo(std::span<const std::string> tokens)
{
    static constexpr size_t MinimumTokenCount = 2; // tag, count
    static constexpr size_t SensorValueCount = 3; // id, nodeId, input
    static constexpr size_t RequiredValueCount = 1 + 5 * SensorValueCount; // id + sensor info

    if (tokens.size() < MinimumTokenCount)
        throw Error("Insufficient number of testpoint-info tokens received.");

    assert(tokens[0] == "<TESTPOINT_INFO>");

    const auto count = to<size_t>(tokens[1]);

    if (count != tokens.size() - MinimumTokenCount)
        throw Error("Missing testpoint-info tokens in received data.");

    std::vector<TestpointInfo> infos(count);

    for (size_t i = 0; i < count; ++i)
    {
        const std::string& token = tokens[2 + i];
        const std::vector<std::string> values = split(token, ValueSeparator);

        if (values.size() != RequiredValueCount)
            throw Error("Invalid number of testpoint-info values received.");

        const std::string& id = values[0];

        if (id.empty())
            throw Error("Empty testpoint ID received.");

        const auto makeValueSpan = [&values](size_t offset) -> std::span<const std::string> {
            return { &values[1 + offset*SensorValueCount], SensorValueCount };
        };

        infos[i].testpointId = id;
        infos[i].conductanceInfo = parseSensorInfo(makeValueSpan(0));
        infos[i].orpInfo = parseSensorInfo(makeValueSpan(1));
        infos[i].phInfo = parseSensorInfo(makeValueSpan(2));
        infos[i].potentiostatInfo = parseSensorInfo(makeValueSpan(3));
        infos[i].temperatureInfo = parseSensorInfo(makeValueSpan(4));
    }

    m_listener->onTestpointInfoReceived(infos);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseHubAlarm(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 2; // tag, values
    static constexpr size_t RequiredValueCount = 2; // type, severity

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of hub-alarm tokens received.");

    assert(tokens[0] == "<HUB_ALARM>");

    const std::vector<std::string> values = split(tokens[1], ValueSeparator);

    if (values.size() != RequiredValueCount)
        throw Error("Invalid number of hub-alarm values received.");

    const auto type = to<AlarmType>(values[0]);
    const auto severity = to<AlarmSeverity>(values[1]);

    m_listener->onHubAlarm(type, severity);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseNodeAlarm(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 3; // tag, id, values
    static constexpr size_t RequiredValueCount = 2; // type, severity

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of node-alarm tokens received.");

    assert(tokens[0] == "<NODE_ALARM>");

    const std::string& id = tokens[1];

    if (id.empty())
        throw Error("Empty node ID received.");

    const std::vector<std::string> values = split(tokens[2], ValueSeparator);

    if (values.size() != RequiredValueCount)
        throw Error("Invalid number of node-alarm values received.");

    const auto type = to<AlarmType>(values[0]);
    const auto severity = to<AlarmSeverity>(values[1]);

    m_listener->onNodeAlarm(id, type, severity);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseHubStatus(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 2; // tag, temperature

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of hub-temperature tokens received.");

    assert(tokens[0] == "<HUB_STATUS>");

    const auto temperature = toDouble(tokens[1]);
    m_listener->onHubStatusReceived(temperature);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseNodeStatus(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 3; // tag, id, values
    static constexpr size_t RequiredValueCount = 3; // voltage, current, temperature

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of node-status tokens received.");

    assert(tokens[0] == "<NODE_STATUS>");

    const std::string& id = tokens[1];

    if (id.empty())
        throw Error("Empty node ID received.");

    const std::vector<std::string> values = split(tokens[2], ValueSeparator);

    if (values.size() != RequiredValueCount)
        throw Error("Invalid number of power values received.");

    const auto voltage = toDouble(values[0]);
    const auto current = toDouble(values[1]);
    const auto temperature = toDouble(values[2]);

    m_listener->onNodeStatusReceived(id, voltage, current, temperature);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseStatus(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 2; // tag, status

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of status tokens received.");

    assert(tokens[0] == "<STATUS>");

    const std::string& token = tokens[1];
    auto status = Status::MeasurementError;

    if (token == "running")
        status = Status::MeasurementStarted;
    else if (token == "idle")
        status = Status::MeasurementStopped;

    m_listener->onStatusChanged(status);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseError(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 2; // tag, message

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of error tokens received.");

    assert(tokens[0] == "<ERROR>");

    const std::string& message = tokens[1];
    m_listener->onError(message);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseVoltammogram(std::span<const std::string> tokens)
{
    static auto convertData = [](const std::string& s) -> std::vector<double>
    {
        std::vector<double> result;

        std::istringstream stream(s);
        std::string token;

        while (std::getline(stream, token, ValueSeparator))
            result.push_back(toDouble(token));

        return result;
    };

    static constexpr size_t RequiredTokenCount = 4; // tag, id, voltage, current

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of voltammogram tokens received.");

    assert(tokens[0] == "<VOLTAMMOGRAM>");

    const std::string& id = tokens[1];

    const std::vector<double> voltage = convertData(tokens[2]);
    const std::vector<double> current = convertData(tokens[3]);

    if (voltage.size() != current.size())
        throw Error("Invalid voltammogram received.");

    m_listener->onVoltammogramReceived(id, voltage, current);
}

// ---------------------------------------------------------------------------------------------- //

void TcpClient::parseSensorData(std::span<const std::string> tokens)
{
    static constexpr size_t RequiredTokenCount = 3; // tag, id, values

    if (tokens.size() != RequiredTokenCount)
        throw Error("Invalid number of sensor-data tokens received.");

    const std::string& tag = tokens[0];
    const std::string& id = tokens[1];

    const std::vector<std::string> values = split(tokens[2], ValueSeparator);

    if (tag == "<CONDUCTANCE>")
    {
        static constexpr size_t RequiredValueCount = 3;

        if (values.size() != RequiredValueCount)
            throw Error("Invalid number of conductance values received.");

        const auto voltage    = toDouble(values[0]);
        const auto current    = toDouble(values[1]);
        const auto admittance = toDouble(values[2]);

        m_listener->onConductanceReceived(id, voltage, current, admittance);
    }
    else if (tag == "<ORP>" || tag == "<PH>" || tag == "<TEMPERATURE>")
    {
        static constexpr size_t RequiredValueCount = 1;

        if (values.size() != RequiredValueCount)
            throw Error("Invalid number of sensor values received.");

        const auto value = toDouble(values[0]);

        if (tag == "<ORP>")
            m_listener->onOrpValueReceived(id, value);
        else if (tag == "<PH>")
            m_listener->onPhValueReceived(id, value);
        else if (tag == "<TEMPERATURE>")
            m_listener->onTemperatureReceived(id, value);
    }
    else
        throw Error("Unknown tag " + tag + " received.");
}

// ---------------------------------------------------------------------------------------------- //

inline
void TcpClient::sendData(const std::string& data)
{
    m_socket.sendData(data);
}

// ---------------------------------------------------------------------------------------------- //
