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
#include "ltc2945.h"
#include "main.h"

class PowerMonitor
{
public:
    PowerMonitor();
    ~PowerMonitor();

    void update();

    auto getVoltage() const -> double;
    auto getCurrent() const -> double;
    auto getTemperature() const -> double;

private:
    friend void PowerPeriodElapsedCallback();
    void isr();

    auto getNextTemperature() -> double;

private:
    volatile bool m_periodElapsed = false;
    static PowerMonitor* s_instance;

    class Interface : public Ltc2945::Interface
    {
    private:
        void write(uint8_t address, uint8_t reg, std::span<const uint8_t> data) override;
        void read(uint8_t address, uint8_t reg, std::span<uint8_t> data) override;
    };

    Interface m_interface;
    Ltc2945 m_ltc2945;

    static constexpr size_t AverageCount = 5;
    AveragingBuffer<double, AverageCount> m_voltage;
    AveragingBuffer<double, AverageCount> m_current;
    AveragingBuffer<double, AverageCount> m_temperature;

    ADC_HandleTypeDef* m_adcHandle = Config::PowerAdcHandle;
    TIM_HandleTypeDef* m_timHandle = Config::PowerTimerHandle;
};
