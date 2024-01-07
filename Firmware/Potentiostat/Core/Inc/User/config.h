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

#include "hostinterface.h"
#include "ltc2945.h"
#include "main.h"
#include "max521x.h"

extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;

namespace Config {
    constexpr uint32_t TimerClockFrequency = 80'000'000;

    constexpr TIM_HandleTypeDef* SampleTimerHandle = &htim6;
    constexpr TIM_HandleTypeDef* SignalTimerHandle = &htim2;
    constexpr TIM_HandleTypeDef* PowerTimerHandle = &htim1;
    constexpr TIM_HandleTypeDef* BlinkTimerHandle = &htim16;
    constexpr TIM_HandleTypeDef* DelayTimerHandle = &htim15;

    constexpr SPI_HandleTypeDef* AdcSpiHandle = &hspi1;

    constexpr I2C_HandleTypeDef* DacI2cHandle = &hi2c1;
    constexpr auto DacAddress = Max5215::Address::A0;

    constexpr ADC_HandleTypeDef* PowerAdcHandle = &hadc1;
    constexpr I2C_HandleTypeDef* PowerI2cHandle = &hi2c1;
    constexpr auto PowerAddress = Ltc2945::Address::A8;
    constexpr float PowerShuntResistance = 0.2F;

    constexpr size_t SamplesPerTransfer = HostInterface::MaximumStringSpanSize;
}
