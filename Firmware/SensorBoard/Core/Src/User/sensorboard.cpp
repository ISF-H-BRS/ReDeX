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
#include "sensorboard.h"
#include "timestamp.h"

#include "usbd_desc.h"

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr char TokenSeparator = ' ';
    volatile bool g_updatePeriodElapsed = false;
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    ASSERT(htim == Config::UpdateTimerHandle);
    g_updatePeriodElapsed = true;
}

// ---------------------------------------------------------------------------------------------- //

SensorBoard::SensorBoard()
    : m_hostInterface(this)
{
}

// ---------------------------------------------------------------------------------------------- //

SensorBoard::~SensorBoard()
{
    HAL_TIM_Base_Stop_IT(Config::UpdateTimerHandle);
    HAL_OPAMP_Stop(Config::BusVoltageOpAmpHandle);

    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_SET);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::exec()
{
    HAL_OPAMP_Start(Config::BusVoltageOpAmpHandle);
    HAL_TIM_Base_Start_IT(Config::UpdateTimerHandle);

    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_RESET);

    while (true)
    {
        m_hostInterface.update();

        if (g_updatePeriodElapsed)
        {
            g_updatePeriodElapsed = false;
            m_powerMonitor.update();
            m_sensorManager.update();
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::onHostDataReceived(const String& data)
{
    const size_t tokenCount = data.getTokenCount(TokenSeparator);

    if (tokenCount < 1)
        return;

    const String tag = data.getToken(TokenSeparator, 0);

    if (tag == "<GET_SENSOR_TYPE>")
        protocolGetSensorType(data, tokenCount);
    else if (tag == "<GET_SENSOR_VALUE>")
        protocolGetSensorValue(data, tokenCount);
    else if (tag == "<GET_POWER_VALUES>")
        protocolGetPowerValues();
    else if (tag == "<GET_BOARD_NAME>")
        protocolGetBoardName();
    else if (tag == "<GET_HARDWARE_VERSION>")
        protocolGetHardwareVersion();
    else if (tag == "<GET_FIRMWARE_VERSION>")
        protocolGetFirmwareVersion();
    else if (tag == "<GET_SERIAL_NUMBER>")
        protocolGetSerialNumber();
    else if (tag == "<GET_BUILD_TIMESTAMP>")
        protocolGetBuildTimestamp();
    else
        sendError("UNKNOWN_COMMAND");
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::onHostDataOverflow()
{
    sendError("DATA_OVERFLOW");
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetSensorType(const String& data, size_t tokenCount)
{
    try {
        checkTokenCount(tokenCount, 2);

        const uint8_t index = toIndex(data.getToken(TokenSeparator, 1));
        sendResponse("<SENSOR_TYPE>", Sensor::toString(m_sensorManager.sensorType(index)));
    }
    catch (const std::exception& e) {
        sendError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetSensorValue(const String& data, size_t tokenCount)
{
    try {
        checkTokenCount(tokenCount, 2);

        const uint8_t index = toIndex(data.getToken(TokenSeparator, 1));
        sendResponse("<SENSOR_VALUE>", String().format("%f", m_sensorManager.sensorValue(index)));
    }
    catch (const std::exception& e) {
        sendError(e.what());
    }
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetPowerValues()
{
    const double voltage = m_powerMonitor.getVoltage();
    const double current = m_powerMonitor.getCurrent();
    const double temperature = m_powerMonitor.getTemperature();

    sendResponse("<POWER_VALUES>", String().format("%f;%f;%f", voltage, current, temperature));
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetBoardName()
{
    sendResponse("<BOARD_NAME>", Config::BoardName);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetHardwareVersion()
{
    sendResponse("<HARDWARE_VERSION>", Config::HardwareVersion);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetFirmwareVersion()
{
    sendResponse("<FIRMWARE_VERSION>", Config::FirmwareVersion);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetSerialNumber()
{
    uint16_t size = 0;
    const uint8_t* buffer = FS_Desc.GetSerialStrDescriptor(USBD_SPEED_FULL, &size);

    String serial;

    // Stored as UTF-16, but contains only ASCII.
    // The first two bytes contain size and type.
    for (size_t i = 2; i < size; i += 2)
        serial += buffer[i];

    sendResponse("<SERIAL_NUMBER>", serial);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::protocolGetBuildTimestamp()
{
    sendResponse("<BUILD_TIMESTAMP>", String().format("%d", BUILD_TIMESTAMP));
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::checkTokenCount(size_t tokenCount, size_t expectedCount)
{
    if (tokenCount < expectedCount)
        throw MissingArgumentError();
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::sendResponse(const String& tag, const String& data)
{
    auto response = tag;

    if (!data.empty())
        response += " " + data;

    m_hostInterface.sendData(response);
}

// ---------------------------------------------------------------------------------------------- //

void SensorBoard::sendError(const String& error)
{
    m_hostInterface.sendData("<ERROR> " + error);
}

// ---------------------------------------------------------------------------------------------- //

auto SensorBoard::toIndex(const String& s) -> size_t
{
    if (s.empty())
        throw InvalidArgumentError();

    auto index = s.toULong();

    if (index >= SensorManager::SensorCount)
        throw InvalidArgumentError();

    return static_cast<size_t>(index);
}

// ---------------------------------------------------------------------------------------------- //
