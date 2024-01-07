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

#include "alarm.h"
#include "assert.h"

#include "usbd_cdc_if.h"

// ---------------------------------------------------------------------------------------------- //

Alarm* Alarm::s_instance = nullptr;

// ---------------------------------------------------------------------------------------------- //

Alarm::Alarm()
{
    ASSERT(s_instance == nullptr);
    s_instance = this;

    CDC_RegisterReceiveCallback(&Alarm::receiveCallback);
}

// ---------------------------------------------------------------------------------------------- //

Alarm::~Alarm()
{
    ASSERT(s_instance == this);
    s_instance = nullptr;

    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STATUS_BAD_GPIO_Port, STATUS_BAD_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(SIGNAL_GREEN_GPIO_Port, SIGNAL_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SIGNAL_RED_GPIO_Port, SIGNAL_RED_Pin, GPIO_PIN_RESET);

    HAL_TIM_Base_Stop(m_timerHandle);
    HAL_TIM_Base_Stop(m_pwmHandle);
    HAL_TIM_Base_Stop(m_signalHandle);
}

// ---------------------------------------------------------------------------------------------- //

void Alarm::exec()
{
    static constexpr uint32_t HeartbeatTimeout = 2000;

    HAL_GPIO_WritePin(STATUS_GOOD_GPIO_Port, STATUS_GOOD_Pin, GPIO_PIN_SET);

    HAL_TIM_Base_Start(m_timerHandle);
    HAL_TIM_Base_Start(m_signalHandle);

    while (true)
    {
        if (__HAL_TIM_GET_COUNTER(m_timerHandle) >= HeartbeatTimeout)
            m_state = State::Disconnected;

        updateSignal();
    }
}

// ---------------------------------------------------------------------------------------------- //

void Alarm::processData(uint8_t* buffer, uint32_t size)
{
    static constexpr const char* LineTerminator = "\r\n";
    static constexpr size_t LineTerminatorSize = 2;

    if (m_currentData.size() + size > m_currentData.capacity())
    {
        m_currentData.clear();
        return;
    }

    for (uint32_t i = 0; i < size; ++i)
    {
        m_currentData += buffer[i];

        if (m_currentData.endsWith(LineTerminator))
        {
            m_currentData.trim(LineTerminatorSize);

            parseCurrentData();
            m_currentData.clear();
        }
    }
}

// ---------------------------------------------------------------------------------------------- //

void Alarm::parseCurrentData()
{
    if (m_currentData == "<IDLE>")
        m_state = State::Idle;
    else if (m_currentData == "<ACTIVE>")
        m_state = State::Active;
    else if (m_currentData == "<ALARM>")
        m_state = State::Alarm;

    __HAL_TIM_SET_COUNTER(m_timerHandle, 0);
}

// ---------------------------------------------------------------------------------------------- //

void Alarm::updateSignal()
{
    static constexpr uint32_t BlinkPeriod = 1000;
    static constexpr uint32_t AlarmPeriod = 250;

    const uint32_t counterValue = __HAL_TIM_GET_COUNTER(m_signalHandle);

    bool greenEnabled = false;
    bool redEnabled = false;

    bool pwmEnabled = false;

    switch (m_state)
    {
    case State::Disconnected:
        redEnabled = counterValue < BlinkPeriod/2;
        break;

    case State::Idle:
        greenEnabled = true;
        break;

    case State::Active:
        greenEnabled = counterValue < BlinkPeriod/2;
        break;

    case State::Alarm:
    {
        const uint32_t alarmValue = counterValue % AlarmPeriod;
        pwmEnabled = counterValue < BlinkPeriod/2;
        redEnabled = pwmEnabled && alarmValue < AlarmPeriod/2;
        break;
    }

    default:
        break;
    }

    const GPIO_PinState greenState = greenEnabled ? GPIO_PIN_SET : GPIO_PIN_RESET;
    const GPIO_PinState redState = redEnabled ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(SIGNAL_GREEN_GPIO_Port, SIGNAL_GREEN_Pin, greenState);
    HAL_GPIO_WritePin(SIGNAL_RED_GPIO_Port, SIGNAL_RED_Pin, redState);

    const bool pwmRunning =
            TIM_CHANNEL_STATE_GET(m_pwmHandle, m_pwmChannel) == HAL_TIM_CHANNEL_STATE_BUSY;

    if (pwmEnabled && !pwmRunning)
        HAL_TIM_PWM_Start(m_pwmHandle, m_pwmChannel);
    else if (!pwmEnabled && pwmRunning)
        HAL_TIM_PWM_Stop(m_pwmHandle, m_pwmChannel);
}

// ---------------------------------------------------------------------------------------------- //

void Alarm::receiveCallback(uint8_t* buffer, uint32_t size)
{
    s_instance->processData(buffer, size);
}

// ---------------------------------------------------------------------------------------------- //

namespace {
    std::array<std::byte, sizeof(Alarm)> g_alarmBuffer;
}

// ---------------------------------------------------------------------------------------------- //

void alarm_main()
{
    auto alarm = new (g_alarmBuffer.data()) Alarm;
    alarm->exec();
    alarm->~Alarm();
}

// ---------------------------------------------------------------------------------------------- //
