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
#include "commandparser.h"
#include "signalgenerator.h"

// ---------------------------------------------------------------------------------------------- //

namespace {
    constexpr auto MinimumFrequency = SignalGenerator::MinimumFrequency;
    constexpr auto MaximumFrequency = SignalGenerator::MaximumFrequency;

    constexpr auto MinimumAmplitude = SignalGenerator::MinimumAmplitude;
    constexpr auto MaximumAmplitude = SignalGenerator::MaximumAmplitude;
}

// ---------------------------------------------------------------------------------------------- //

namespace Command {
    constexpr uint32_t NoCommand   = 0x00;
    constexpr uint32_t SetupSignal = 0x01;
    constexpr uint32_t SetGain     = 0x02;
    constexpr uint32_t Reset       = 0x07;

    constexpr uint32_t CmdOffset = 29;
    constexpr uint32_t CmdMask   = 0x00000007;

    constexpr uint32_t WaveformOffset = 26;
    constexpr uint32_t WaveformMask   = 0x00000007;

    constexpr uint32_t FrequencyOffset = 16;
    constexpr uint32_t FrequencyMask   = 0x000003ff;

    constexpr uint32_t AmplitudeOffset = 0;
    constexpr uint32_t AmplitudeMask   = 0x000000ff;

    constexpr uint32_t GainOffset = 0;
    constexpr uint32_t GainMask   = 0x00000003;
}

// ---------------------------------------------------------------------------------------------- //

CommandParser::CommandParser(Owner* owner)
    : m_owner(owner)
{
    ASSERT(owner != nullptr);
}

// ---------------------------------------------------------------------------------------------- //

void CommandParser::parse(uint32_t command)
{
    const uint32_t cmd = (command >> Command::CmdOffset) & Command::CmdMask;

    if (cmd == Command::SetupSignal)
    {
        const uint32_t waveform = (command >> Command::WaveformOffset) & Command::WaveformMask;

        if (waveform >= WaveformCount)
            return;

        const uint32_t frequency = (command >> Command::FrequencyOffset) & Command::FrequencyMask;

        if (frequency < MinimumFrequency || frequency > MaximumFrequency)
            return;

        const double amplitude =
            ((command >> Command::AmplitudeOffset) & Command::AmplitudeMask) * 0.01;

        if (amplitude < MinimumAmplitude || amplitude > MaximumAmplitude)
            return;

        m_owner->onCommandSetupSignal(static_cast<Waveform>(waveform), frequency, amplitude);
    }
    else if (cmd == Command::SetGain)
    {
        const uint32_t gain = (command >> Command::GainOffset) & Command::GainMask;

        if (gain >= GainCount)
            return;

        m_owner->onCommandSetGain(static_cast<Gain>(gain));
    }
    else if (cmd == Command::Reset)
        m_owner->onCommandReset();
}

// ---------------------------------------------------------------------------------------------- //
