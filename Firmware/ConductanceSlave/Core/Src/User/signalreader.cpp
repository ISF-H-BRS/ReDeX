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
#include "signalreader.h"

// ---------------------------------------------------------------------------------------------- //

SignalReader::SignalReader()
{
    HAL_ADCEx_Calibration_Start(m_voltageAdcHandle, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(m_currentAdcHandle, ADC_SINGLE_ENDED);

    HAL_ADC_Start(m_voltageAdcHandle);
    HAL_ADC_Start(m_currentAdcHandle);

    __HAL_TIM_ENABLE(m_triggerHandle);
    __HAL_TIM_DISABLE(m_triggerHandle);
}

// ---------------------------------------------------------------------------------------------- //

SignalReader::~SignalReader()
{
    HAL_ADC_Stop(m_voltageAdcHandle);
    HAL_ADC_Stop(m_currentAdcHandle);
}

// ---------------------------------------------------------------------------------------------- //

void SignalReader::update()
{
    // TRGO is used as external ADC trigger,
    // timer CNT_EN is used as trigger event

    static constexpr uint16_t MaximumValue = 0xfff0; // 256x oversample, 4-bit shift

    HAL_StatusTypeDef status = {};

    status = HAL_ADC_PollForConversion(m_voltageAdcHandle, 1);
    ASSERT(status == HAL_OK);

    status = HAL_ADC_PollForConversion(m_currentAdcHandle, 1);
    ASSERT(status == HAL_OK);

    __HAL_TIM_DISABLE(m_triggerHandle);

    m_voltageValue = HAL_ADC_GetValue(m_voltageAdcHandle);
    m_currentValue = HAL_ADC_GetValue(m_currentAdcHandle);

    __HAL_TIM_ENABLE(m_triggerHandle);

    ASSERT(m_voltageValue <= MaximumValue && m_currentValue <= MaximumValue);

    // Undo TIA inversion
    m_currentValue = MaximumValue - m_currentValue;
}

// ---------------------------------------------------------------------------------------------- //

auto SignalReader::voltageValue() const -> uint16_t
{
    return m_voltageValue;
}

// ---------------------------------------------------------------------------------------------- //

auto SignalReader::currentValue() const -> uint16_t
{
    return m_currentValue;
}

// ---------------------------------------------------------------------------------------------- //
