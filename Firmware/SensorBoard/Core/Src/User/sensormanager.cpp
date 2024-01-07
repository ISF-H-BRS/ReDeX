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

#include "assert.h"
#include "config.h"
#include "sensormanager.h"

// ---------------------------------------------------------------------------------------------- //

SensorManager::SensorManager()
{
    static constexpr std::array<I2C_HandleTypeDef*, SensorCount> SensorHandles = {
        Config::Sensor1Handle, Config::Sensor2Handle
    };

    for (size_t i = 0; i < SensorCount; ++i)
    {
        ASSERT(m_sensors[i] == nullptr);

        char* buffer = m_memPool.getNextFreeBuffer();
        I2C_HandleTypeDef* handle = SensorHandles[i];

        const Sensor::Type type = readSensorType(i);

        switch (type)
        {
        case Sensor::Type::pH_ORP:
            m_sensors[i] = new (buffer) PhSensor(handle);
            break;

        case Sensor::Type::Temperature:
            m_sensors[i] = new (buffer) TemperatureSensor(handle);
            break;

        default:
            m_sensors[i] = new (buffer) NoneSensor();
            break;
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

SensorManager::~SensorManager()
{
    for (auto& sensor : m_sensors)
    {
        if (sensor)
            sensor->~Sensor();

        sensor = nullptr;
    }
}

// ---------------------------------------------------------------------------------------------- //

auto SensorManager::sensorType(uint8_t sensor) const -> Sensor::Type
{
    ASSERT(sensor < SensorCount);
    return m_sensors[sensor]->type();
}

// ---------------------------------------------------------------------------------------------- //

auto SensorManager::sensorValue(uint8_t sensor) const -> double
{
    ASSERT(sensor < SensorCount);
    return m_sensors[sensor]->value();
}

// ---------------------------------------------------------------------------------------------- //

void SensorManager::update()
{
    for (auto sensor : m_sensors)
        sensor->update();
}

// ---------------------------------------------------------------------------------------------- //

auto SensorManager::readSensorType(uint8_t sensor) const -> Sensor::Type
{
    ASSERT(sensor < SensorCount);

    struct Input
    {
        GPIO_TypeDef* port;
        uint32_t pin;
    };

    static constexpr std::array<std::array<Input, 2>, SensorCount> Inputs = {{
        {{{ ID1_0_GPIO_Port, ID1_0_Pin }, { ID1_1_GPIO_Port, ID1_1_Pin }}},
        {{{ ID2_0_GPIO_Port, ID2_0_Pin }, { ID2_1_GPIO_Port, ID2_1_Pin }}}
    }};

    const Input& input0 = Inputs[sensor][0];
    const Input& input1 = Inputs[sensor][1];

    const GPIO_PinState state0 = HAL_GPIO_ReadPin(input0.port, input0.pin);
    const GPIO_PinState state1 = HAL_GPIO_ReadPin(input1.port, input1.pin);

    const unsigned int type = ((state1 == GPIO_PIN_SET ? 1 : 0) << 1) |
                              ((state0 == GPIO_PIN_SET ? 1 : 0) << 0);

    return static_cast<Sensor::Type>(type);
}

// ---------------------------------------------------------------------------------------------- //
