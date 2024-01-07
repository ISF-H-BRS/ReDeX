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
#include "temperaturesensor.h"

#include <cmath>

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr SC18IS602B::Setup BridgeSetup = {
        SC18IS602B::Address::A0,
        SC18IS602B::BitOrder::MsbFirst,
        SC18IS602B::SpiMode::IdleLowLeadingEdge,
        SC18IS602B::ClockRate::_1843kHz
    };

    constexpr MAX31865::Setup SensorSetup = {
        MAX31865::ConnectionMode::TwoOrFourWire,
        MAX31865::FilterSelect::_50Hz
    };

    constexpr auto SensorSlaveSelect = SC18IS602B::SlaveSelect::SS1;
    constexpr auto SensorDataReady = SC18IS602B::SlaveSelect::SS2;
}

// ---------------------------------------------------------------------------------------------- //

TemperatureSensor::TemperatureSensor(I2C_HandleTypeDef* i2c)
    : m_i2c(i2c),
      m_bridgeInterface(m_i2c),
      m_bridge(BridgeSetup, &m_bridgeInterface),
      m_sensorInterface(&m_bridge),
      m_sensor(SensorSetup, &m_sensorInterface)
{
    m_bridge.setGpioEnabled(false, false, true, false);
    m_bridge.setGpioConfiguration({}, {}, SC18IS602B::GpioConfiguration::InputOnly, {});

    m_sensor.setBiasVoltageEnabled(true);
    m_sensor.setAutoConversionEnabled(true);
}

// ---------------------------------------------------------------------------------------------- //

auto TemperatureSensor::type() const -> Type
{
    return Type::Temperature;
}

// ---------------------------------------------------------------------------------------------- //

auto TemperatureSensor::value() const -> double
{
    return m_value.getValue();
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureSensor::update()
{
    static constexpr double R = 1000.0;

    static constexpr double A = 3.9083e-3;
    static constexpr double B = -5.775e-7;

    static constexpr double RA = R*A;
    static constexpr double RB = R*B;

    static constexpr double RA2 = R*R*A*A;

    static constexpr double ReferenceResistance = 4020.0;

    const double resistance = m_sensor.getRelativeValue() * ReferenceResistance;
    m_value.addSample((-RA + std::sqrt(RA2 - 4.0 * RB * (R - resistance))) / (2.0 * RB));
}

// ---------------------------------------------------------------------------------------------- //

TemperatureSensor::BridgeInterface::BridgeInterface(I2C_HandleTypeDef* i2c)
    : m_i2c(i2c)
{
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureSensor::BridgeInterface::read(uint8_t address, std::span<uint8_t> data) const
{
    HAL_I2C_Master_Receive(m_i2c, address, data.data(), data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureSensor::BridgeInterface::write(uint8_t address, std::span<const uint8_t> data) const
{
    auto ptr = const_cast<uint8_t*>(data.data());
    HAL_I2C_Master_Transmit(m_i2c, address, ptr, data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //

TemperatureSensor::SensorInterface::SensorInterface(SC18IS602B* bridge)
    : m_bridge(bridge)
{
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureSensor::SensorInterface::write(std::span<const uint8_t> data) const
{
    m_bridge->writeData(SensorSlaveSelect, data);
}

// ---------------------------------------------------------------------------------------------- //

void TemperatureSensor::SensorInterface::swap(std::span<const uint8_t> out,
                                              std::span<uint8_t> in) const
{
    m_bridge->swapData(SensorSlaveSelect, out, in);
}

// ---------------------------------------------------------------------------------------------- //

auto TemperatureSensor::SensorInterface::dataReady() const -> bool
{
    HAL_Delay(1);
    return m_bridge->getGpioValue(SensorDataReady) == false;
}

// ---------------------------------------------------------------------------------------------- //
