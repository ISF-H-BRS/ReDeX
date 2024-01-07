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

#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;

extern OPAMP_HandleTypeDef hopamp1;

extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim6;

namespace Config {
    constexpr const char* BoardName = "I2CCarrier";
    constexpr const char* HardwareVersion = "1.0";
    constexpr const char* FirmwareVersion = "1.0";

    constexpr I2C_HandleTypeDef* Sensor1Handle = &hi2c3;
    constexpr I2C_HandleTypeDef* Sensor2Handle = &hi2c1;

    constexpr OPAMP_HandleTypeDef* BusVoltageOpAmpHandle = &hopamp1;

    constexpr ADC_HandleTypeDef* PowerMonitorHandle = &hadc1;
    constexpr TIM_HandleTypeDef* UpdateTimerHandle = &htim1;

    constexpr TIM_HandleTypeDef* DelayTimerHandle = &htim6;
}
