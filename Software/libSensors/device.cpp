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

#include "device.h"
#include "serialport.h"

#include <map>
#include <sstream>

// ---------------------------------------------------------------------------------------------- //

using namespace isf::Sensors;

// ---------------------------------------------------------------------------------------------- //

namespace {
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

class InvalidResponseError : public Device::Error
{
public:
    InvalidResponseError(const std::string& response)
        : Device::Error("Invalid response received from device: '" + response + "'") {}
};

// ---------------------------------------------------------------------------------------------- //

class Device::Private
{
public:
    static constexpr std::chrono::milliseconds DefaultTimeout = SerialPort::DefaultTimeout;

public:
    Private(const std::string& port);

    auto sendRequest(std::string request,
                     std::chrono::milliseconds timeout = DefaultTimeout) const -> std::string;

    static auto parseString(const std::string& response,
                            const std::string& expectedTag) -> std::string;

    static auto parseULong(const std::string& response,
                           const std::string& expectedTag) -> unsigned long;

    static auto parseDouble(const std::string& response,
                            const std::string& expectedTag) -> double;

    static auto parseSensorType(const std::string& response,
                                const std::string& expectedTag) -> SensorType;

    static auto parsePowerValues(const std::string& response,
                                 const std::string& expectedTag) -> PowerValues;
private:
    static void checkError(const std::string& response);
    static auto mapError(const std::string& error) -> std::string;

private:
    SerialPort m_port;
};

// ---------------------------------------------------------------------------------------------- //

Device::Device(const std::string& port)
    : d(std::make_unique<Private>(port))
{
}

// ---------------------------------------------------------------------------------------------- //

Device::~Device() = default;

// ---------------------------------------------------------------------------------------------- //

auto Device::getSensorType(size_t index) const -> SensorType
{
    const std::string response = d->sendRequest("<GET_SENSOR_TYPE> " + toString(index));
    return Private::parseSensorType(response, "<SENSOR_TYPE>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getSensorValue(size_t index) const -> double
{
    const std::string response = d->sendRequest("<GET_SENSOR_VALUE> " + toString(index));
    return Private::parseDouble(response, "<SENSOR_VALUE>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getPowerValues() const -> PowerValues
{
    const std::string response = d->sendRequest("<GET_POWER_VALUES>");
    return Private::parsePowerValues(response, "<POWER_VALUES>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getHardwareVersion() const -> std::string
{
    const std::string response = d->sendRequest("<GET_HARDWARE_VERSION>");
    return Private::parseString(response, "<HARDWARE_VERSION>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getFirmwareVersion() const -> std::string
{
    const std::string response = d->sendRequest("<GET_FIRMWARE_VERSION>");
    return Private::parseString(response, "<FIRMWARE_VERSION>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getSerialNumber() const -> std::string
{
    const std::string response = d->sendRequest("<GET_SERIAL_NUMBER>");
    return Private::parseString(response, "<SERIAL_NUMBER>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::getBuildTimestamp() const -> unsigned long
{
    const std::string response = d->sendRequest("<GET_BUILD_TIMESTAMP>");
    return Private::parseULong(response, "<BUILD_TIMESTAMP>");
}

// ---------------------------------------------------------------------------------------------- //

auto Device::toPhValue(double voltage, double temperature) -> double
{
    const double slope = -0.0001984845344212229146 * (temperature + 273.15);
    return 7.0 + voltage / slope;
}

// ---------------------------------------------------------------------------------------------- //

Device::Private::Private(const std::string& port)
    : m_port(port) {}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::sendRequest(std::string request,
                                  std::chrono::milliseconds timeout) const -> std::string
{
    request += "\r\n";
    m_port.sendData(request);

    const bool dataAvailable = m_port.waitForDataAvailable(timeout);

    if (!dataAvailable)
        throw Error("Request timed out.");

    const std::vector<char> data = m_port.readAllData();
    std::string response(data.begin(), data.end());

    static const std::string lineBreak = "\r\n";

    if (!response.ends_with(lineBreak))
        throw InvalidResponseError(response);

    response.erase(response.size() - lineBreak.size());

    checkError(response);
    return response;
}

// ---------------------------------------------------------------------------------------------- //

void Device::Private::checkError(const std::string& response)
{
    const std::string tag = response.substr(0, response.find_first_of(' '));

    if (tag == "<ERROR>")
    {
        const std::string error = response.substr(tag.length() + 1);
        throw Error(mapError(error));
    }
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::parseString(const std::string& response,
                                  const std::string& expectedTag) -> std::string
{
    const std::vector<std::string> tokens = split(response, ' ');

    if (tokens.size() == 2 && tokens.at(0) == expectedTag)
        return tokens.at(1);

    throw InvalidResponseError(response);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::parseULong(const std::string& response,
                                 const std::string& expectedTag) -> unsigned long
{
    const std::string value = parseString(response, expectedTag);

    try {
        return to<unsigned long>(value);
    }
    catch (...) {}

    throw InvalidResponseError(response);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::parseDouble(const std::string& response,
                                  const std::string& expectedTag) -> double
{
    const std::string value = parseString(response, expectedTag);

    try {
        return to<double>(value);
    }
    catch (...) {}

    throw InvalidResponseError(response);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::parseSensorType(const std::string& response,
                                      const std::string& expectedTag) -> SensorType
{
    static const std::map<std::string, SensorType> types = {
        { "Unknown",     SensorType::Unknown     },
        { "pH/ORP",      SensorType::pH_ORP      },
        { "Temperature", SensorType::Temperature },
        { "None",        SensorType::None        }
    };

    const std::string type = parseString(response, expectedTag);

    auto it = types.find(type.c_str());

    if (it == types.end())
        throw InvalidResponseError(response);

    return it->second;
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::parsePowerValues(const std::string& response,
                                       const std::string& expectedTag) -> PowerValues
{
    const std::string power = parseString(response, expectedTag);

    const std::vector<std::string> values = split(power, ';');

    if (values.size() == 3)
    {
        try {
            return {
                to<double>(values.at(0)),
                to<double>(values.at(1)),
                to<double>(values.at(2))
            };
        }
        catch (...) {}
    }

    throw InvalidResponseError(response);
}

// ---------------------------------------------------------------------------------------------- //

auto Device::Private::mapError(const std::string& error) -> std::string
{
    // TODO: Create shared header

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
