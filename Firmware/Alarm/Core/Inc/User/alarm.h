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

#ifdef __cplusplus

#include "defaultstring.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

class Alarm
{
public:
    Alarm();
    ~Alarm();

    void exec();

private:
    void processData(uint8_t* buffer, uint32_t size);
    void parseCurrentData();

    void updateSignal();

    static void receiveCallback(uint8_t* buffer, uint32_t size);

private:
    enum class State {
        Disconnected,
        Idle,
        Active,
        Alarm
    } m_state = State::Disconnected;

    String m_currentData;

    TIM_HandleTypeDef* m_timerHandle = &htim1;
    TIM_HandleTypeDef* m_pwmHandle = &htim2;
    TIM_HandleTypeDef* m_signalHandle = &htim3;

    const uint32_t m_pwmChannel = TIM_CHANNEL_1;

    static Alarm* s_instance;
};

extern "C" void alarm_main();
#else
void alarm_main(void);
#endif
