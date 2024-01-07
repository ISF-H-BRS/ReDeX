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
#include "blinker.h"
#include "config.h"

// ---------------------------------------------------------------------------------------------- //

void BlinkPeriodElapsedCallback()
{
    ASSERT(Blinker::s_instance != nullptr);
    Blinker::s_instance->isr();
}

// ---------------------------------------------------------------------------------------------- //

Blinker* Blinker::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

Blinker::Blinker(GPIO_TypeDef* port, uint16_t pin)
    : m_port(port),
      m_pin(pin)
{
    ASSERT(s_instance == nullptr);
    s_instance = this;
}

// ---------------------------------------------------------------------------------------------- //

Blinker::~Blinker()
{
    HAL_TIM_Base_Stop_IT(Config::BlinkTimerHandle);

    ASSERT(s_instance == this);
    s_instance = nullptr;
}

// ---------------------------------------------------------------------------------------------- //

void Blinker::start()
{
    HAL_TIM_Base_Start_IT(Config::BlinkTimerHandle);
}

// ---------------------------------------------------------------------------------------------- //

void Blinker::stop()
{
    HAL_TIM_Base_Stop_IT(Config::BlinkTimerHandle);
    HAL_GPIO_WritePin(m_port, m_pin, GPIO_PIN_RESET);
}

// ---------------------------------------------------------------------------------------------- //

void Blinker::update()
{
    if (m_periodElapsed)
    {
        m_periodElapsed = false;
        m_state = !m_state;

        const GPIO_PinState state = m_state ? GPIO_PIN_RESET : GPIO_PIN_SET;
        HAL_GPIO_WritePin(m_port, m_pin, state);
    }
}

// ---------------------------------------------------------------------------------------------- //

void Blinker::isr()
{
    m_periodElapsed = true;
}

// ---------------------------------------------------------------------------------------------- //
