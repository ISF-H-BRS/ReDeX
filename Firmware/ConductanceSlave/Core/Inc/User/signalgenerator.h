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

#include "config.h"
#include "global.h"

class SignalGenerator
{
public:
    using Real = float;

    static constexpr unsigned int MinimumFrequency = 1;
    static constexpr unsigned int MaximumFrequency = 1000;

    static constexpr Real MinimumAmplitude = 0.0;
    static constexpr Real MaximumAmplitude = 1.5;

    static constexpr Real ReferenceVoltage = 3.0;

    static constexpr unsigned int SampleRate = 10000;

public:
    SignalGenerator();
    ~SignalGenerator();

    void reset();
    void setup(Waveform waveform, unsigned int frequency, Real amplitude);
    void update();

private:
    void setValue(uint32_t channel, uint32_t value);

private:
    DAC_HandleTypeDef* m_dacHandle = Config::DacHandle;

    Waveform m_waveform = Waveform::None;
    Real m_frequency = 1.0;
    Real m_amplitude = 0.0;

    unsigned int m_signalPosition = 0;
};
