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
#include "phsensor.h"

// ---------------------------------------------------------------------------------------------- //

PhSensor::PhSensor(I2C_HandleTypeDef* i2c)
    : m_sensorInterface(i2c),
      m_sensor(ADS1110::Address::A0, &m_sensorInterface)
{
}

// ---------------------------------------------------------------------------------------------- //

auto PhSensor::type() const -> Type
{
    return Type::pH_ORP;
}

// ---------------------------------------------------------------------------------------------- //

auto PhSensor::value() const -> double
{
    return m_value.getValue();
}

// ---------------------------------------------------------------------------------------------- //

void PhSensor::update()
{
    static constexpr double ReferenceVoltage = 2.048;

    const int16_t value = m_sensor.readValue();
    m_value.addSample(m_sensor.valueToReal(value) * ReferenceVoltage);
}

// ---------------------------------------------------------------------------------------------- //

PhSensor::SensorInterface::SensorInterface(I2C_HandleTypeDef* i2c)
    : m_i2c(i2c) {}

// ---------------------------------------------------------------------------------------------- //

void PhSensor::SensorInterface::read(uint8_t address, std::span<uint8_t> data) const
{
    HAL_I2C_Master_Receive(m_i2c, address, data.data(), data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //

void PhSensor::SensorInterface::write(uint8_t address, std::span<const uint8_t> data) const
{
    auto ptr = const_cast<uint8_t*>(data.data());
    HAL_I2C_Master_Transmit(m_i2c, address, ptr, data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //
