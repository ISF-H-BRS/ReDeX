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

#include "ads1110.h"
#include "averagingbuffer.h"
#include "sensor.h"

class PhSensor : public Sensor
{
public:
    PhSensor(I2C_HandleTypeDef* i2c);

    auto type() const -> Type override;
    auto value() const -> double override;

    void update() override;

private:
    class SensorInterface : public ADS1110::Interface
    {
    public:
        SensorInterface(I2C_HandleTypeDef* i2c);

    private:
        void read(uint8_t address, std::span<uint8_t> data) const override;
        void write(uint8_t address, std::span<const uint8_t> data) const override;

    private:
        I2C_HandleTypeDef* m_i2c;
    };

private:
    SensorInterface m_sensorInterface;
    ADS1110 m_sensor;

    AveragingBuffer<double, 3> m_value;
};
