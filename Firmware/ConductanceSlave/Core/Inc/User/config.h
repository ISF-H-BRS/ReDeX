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

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern DAC_HandleTypeDef hdac1;

extern OPAMP_HandleTypeDef hopamp2;

extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_rx;

extern TIM_HandleTypeDef htim1;

namespace Config {
    constexpr const char* BoardName = "ConductanceSlave";
    constexpr const char* HardwareVersion = "3.1";
    constexpr const char* FirmwareVersion = "3.1";

    constexpr ADC_HandleTypeDef* VoltageAdcHandle = &hadc1;
    constexpr ADC_HandleTypeDef* CurrentAdcHandle = &hadc2;

    constexpr TIM_HandleTypeDef* AdcTriggerTimHandle = &htim1;

    constexpr DAC_HandleTypeDef* DacHandle = &hdac1;

    constexpr OPAMP_HandleTypeDef* VoutOpAmpHandle = &hopamp2;

    constexpr SPI_HandleTypeDef* CommSpiHandle   = &hspi2;
    constexpr DMA_HandleTypeDef* CommDmaRxHandle = &hdma_spi2_rx;
}
