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

#include "gainmux.h"
#include "main.h"

// ---------------------------------------------------------------------------------------------- //

GainMux::GainMux()
{
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MUX_EN_GPIO_Port, MUX_EN_Pin, GPIO_PIN_SET);
}

// ---------------------------------------------------------------------------------------------- //

GainMux::~GainMux()
{
    HAL_GPIO_WritePin(MUX_EN_GPIO_Port, MUX_EN_Pin, GPIO_PIN_RESET);
}

// ---------------------------------------------------------------------------------------------- //

void GainMux::setGain(Gain gain)
{
    static const auto toPinState = [](bool set) {
        return set ? GPIO_PIN_SET : GPIO_PIN_RESET;
    };

    const auto bits = static_cast<uint8_t>(gain);

    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, toPinState(bits & (1<<0)));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, toPinState(bits & (1<<1)));
}

// ---------------------------------------------------------------------------------------------- //
