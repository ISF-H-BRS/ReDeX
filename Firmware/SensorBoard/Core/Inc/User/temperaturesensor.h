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

#include "averagingbuffer.h"
#include "max31865.h"
#include "sc18is602b.h"
#include "sensor.h"

class TemperatureSensor : public Sensor
{
public:
    TemperatureSensor(I2C_HandleTypeDef* i2c);

    auto type() const -> Type override;
    auto value() const -> double override;

    void update() override;

private:
    class BridgeInterface : public SC18IS602B::Interface
    {
    public:
        BridgeInterface(I2C_HandleTypeDef* i2c);

    private:
        void read(uint8_t address, std::span<uint8_t> data) const override;
        void write(uint8_t address, std::span<const uint8_t> data) const override;

    private:
        I2C_HandleTypeDef* m_i2c;
    };

    class SensorInterface : public MAX31865::Interface
    {
    public:
        SensorInterface(SC18IS602B* bridge);

    private:
        void write(std::span<const uint8_t> data) const override;
        void swap(std::span<const uint8_t> out, std::span<uint8_t> in) const override;
        auto dataReady() const -> bool override;

    private:
        SC18IS602B* m_bridge;
    };

private:
    I2C_HandleTypeDef* m_i2c;

    BridgeInterface m_bridgeInterface;
    SC18IS602B m_bridge;

    SensorInterface m_sensorInterface;
    MAX31865 m_sensor;

    AveragingBuffer<double, 3> m_value;
};
