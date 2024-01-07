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
#include "powermonitor.h"

// ---------------------------------------------------------------------------------------------- //

void PowerPeriodElapsedCallback()
{
    ASSERT(PowerMonitor::s_instance != nullptr);
    PowerMonitor::s_instance->isr();
}

// ---------------------------------------------------------------------------------------------- //

PowerMonitor* PowerMonitor::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

PowerMonitor::PowerMonitor()
    : m_ltc2945(&m_interface, Config::PowerAddress, Config::PowerShuntResistance)
{
    ASSERT(s_instance == nullptr);
    s_instance = this;

    HAL_ADCEx_Calibration_Start(m_adcHandle, ADC_SINGLE_ENDED);
    HAL_ADC_Start(m_adcHandle);

    HAL_TIM_Base_Start_IT(m_timHandle);
}

// ---------------------------------------------------------------------------------------------- //

PowerMonitor::~PowerMonitor()
{
    HAL_TIM_Base_Stop_IT(m_timHandle);
    HAL_ADC_Stop(m_adcHandle);

    ASSERT(s_instance == this);
    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::update()
{
    if (m_periodElapsed)
    {
        m_periodElapsed = false;

        m_voltage.addSample(m_ltc2945.getVoltage());
        m_current.addSample(m_ltc2945.getCurrent());
        m_temperature.addSample(getNextTemperature());
    }
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getVoltage() const -> double
{
    return m_voltage.getValue();
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getCurrent() const -> double
{
    return m_current.getValue();
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getTemperature() const -> double
{
    return m_temperature.getValue();
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::isr()
{
    m_periodElapsed = true;
}

// ---------------------------------------------------------------------------------------------- //

auto PowerMonitor::getNextTemperature() -> double
{
    static constexpr double ReferenceVoltage = 3.3;
    static constexpr double CalibrationVoltage = 3.0;
    static constexpr double CalibrationFactor = ReferenceVoltage / CalibrationVoltage;

    static constexpr double JunctionToAmbient = 5.0; // Rough estimate

    static constexpr uintptr_t TsCal1Address = 0x1fff75a8;
    static constexpr uintptr_t TsCal2Address = 0x1fff75ca;

    static constexpr double TsCal1Temperature =  30.0;
    static constexpr double TsCal2Temperature = 130.0;

    static const double tsCal1Value = *reinterpret_cast<uint16_t*>(TsCal1Address);
    static const double tsCal2Value = *reinterpret_cast<uint16_t*>(TsCal2Address);

    static const double tsCalRatio =
            (TsCal2Temperature - TsCal1Temperature) / (tsCal2Value - tsCal1Value);

    HAL_ADC_PollForConversion(m_adcHandle, 10);

    const double value = HAL_ADC_GetValue(m_adcHandle) * CalibrationFactor;

    HAL_ADC_Start(m_adcHandle);

    return (value - tsCal1Value) * tsCalRatio + TsCal1Temperature - JunctionToAmbient;
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::Interface::write(uint8_t address, uint8_t reg, std::span<const uint8_t> data)
{
    auto i2c = Config::PowerI2cHandle;
    auto ptr = const_cast<uint8_t*>(data.data());

    HAL_I2C_Mem_Write(i2c, address, reg, I2C_MEMADD_SIZE_8BIT, ptr, data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //

void PowerMonitor::Interface::read(uint8_t address, uint8_t reg, std::span<uint8_t> data)
{
    auto i2c = Config::PowerI2cHandle;
    HAL_I2C_Mem_Read(i2c, address, reg, I2C_MEMADD_SIZE_8BIT, data.data(), data.size(), 100);
}

// ---------------------------------------------------------------------------------------------- //
