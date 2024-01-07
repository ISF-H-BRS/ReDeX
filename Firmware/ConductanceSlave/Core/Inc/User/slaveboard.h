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

#include "commandparser.h"
#include "config.h"
#include "gainmux.h"
#include "signalgenerator.h"
#include "signalreader.h"

#include <array>

class SlaveBoard : public CommandParser::Owner
{
public:
    SlaveBoard();
    ~SlaveBoard();

    void exec();

private:
    friend void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin);
    void handleExternalInterruptFalling();

    friend void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin);
    void handleExternalInterruptRising();

    void onCommandSetupSignal(Waveform waveform, unsigned int frequency, double amplitude) override;
    void onCommandSetGain(Gain gain) override;
    void onCommandReset() override;

private:
    SPI_HandleTypeDef* m_commSpiHandle = Config::CommSpiHandle;
    DMA_HandleTypeDef* m_commDmaRxHandle = Config::CommDmaRxHandle;

    uint32_t m_currentCommand = 0;
    volatile uint32_t m_unhandledCommand = 0;

    SignalGenerator m_signalGenerator;
    SignalReader m_signalReader;

    GainMux m_gainMux;

    CommandParser m_commandParser;

    static SlaveBoard* s_instance;
};
